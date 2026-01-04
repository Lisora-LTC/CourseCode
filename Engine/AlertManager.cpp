#include "AlertManager.h"
#include <algorithm>

// ==================== 构造与析构 ====================

AlertManager::AlertManager()
    : highestLevel_(AlertLevel::NORMAL)
{
    // 初始化
}

AlertManager::~AlertManager()
{
    // 清理资源
}

// ==================== 核心检测函数 ====================

AlertLevel AlertManager::checkCondition(const SystemData &data)
{
    // 1. 清空newAlerts_列表
    newAlerts_.clear();

    // 2. 标记所有活跃告警为未更新
    for (auto &alert : activeAlerts_)
    {
        alert.updated = false;
    }

    // 3. 调用各个子检测函数
    checkSensorFaults(data);
    checkFuelAbnormal(data);
    checkThrustAbnormal(data);
    checkTempAbnormal(data);

    // 4. 检查双发失效（最危险的情况）
    bool leftEngineDead = data.leftEngine.state == SystemState::OFF &&
                          data.systemState != SystemState::OFF;
    bool rightEngineDead = data.rightEngine.state == SystemState::OFF &&
                           data.systemState != SystemState::OFF;

    if (leftEngineDead && rightEngineDead)
    {
        addAlert(FaultType::DUAL_ENGINE_FAILURE, AlertLevel::DANGER,
                 "DUAL ENGINE FAILURE", data.timestamp);
    }

    // 5. 移除未更新的告警（即条件不再满足的告警）
    activeAlerts_.erase(
        std::remove_if(activeAlerts_.begin(), activeAlerts_.end(),
                       [](const AlertInfo &a)
                       { return !a.updated; }),
        activeAlerts_.end());

    // 6. 更新最高告警级别
    updateHighestLevel();

    // 7. 返回最高级别
    return highestLevel_;
}

void AlertManager::updateTimers(double dt)
{
    // 1. 遍历activeAlerts_，更新计时器
    for (auto &alert : activeAlerts_)
    {
        if (alert.displayTimer > 0)
        {
            alert.displayTimer -= dt;
            if (alert.displayTimer <= 0)
            {
                alert.isActive = false;
            }
        }
    }

    // 2. 清理不活跃的告警（displayTimer <= 0）
    activeAlerts_.erase(
        std::remove_if(activeAlerts_.begin(), activeAlerts_.end(),
                       [](const AlertInfo &a)
                       { return !a.isActive; }),
        activeAlerts_.end());
}

// ==================== 数据访问接口 ====================

std::vector<std::string> AlertManager::getActiveMessages() const
{
    std::vector<std::string> messages;

    // 遍历activeAlerts_，只返回displayTimer > 0的活跃消息
    for (const auto &alert : activeAlerts_)
    {
        if (alert.isActive && alert.displayTimer > 0)
        {
            messages.push_back(alert.message);
        }
    }

    return messages;
}

std::vector<AlertInfo> AlertManager::getAllAlerts() const
{
    return activeAlerts_;
}

AlertLevel AlertManager::getHighestAlertLevel() const
{
    return highestLevel_;
}

std::vector<AlertInfo> AlertManager::getNewAlerts()
{
    std::vector<AlertInfo> result = newAlerts_;
    newAlerts_.clear();
    return result;
}

void AlertManager::clearAllAlerts()
{
    activeAlerts_.clear();
    newAlerts_.clear();
    lastLogTime_.clear();
    highestLevel_ = AlertLevel::NORMAL;
}

// ==================== 私有检测函数 ====================

void AlertManager::checkSensorFaults(const SystemData &data)
{
    // 检查左发动机传感器
    int leftFaults = 0;
    if (!data.leftEngine.n1SensorValid)
        leftFaults++;
    if (!data.leftEngine.egtSensorValid)
        leftFaults++;

    // 检查右发动机传感器
    int rightFaults = 0;
    if (!data.rightEngine.n1SensorValid)
        rightFaults++;
    if (!data.rightEngine.egtSensorValid)
        rightFaults++;

    // 左发N1传感器故障
    if (!data.leftEngine.n1SensorValid)
    {
        addAlert(FaultType::SENSOR_FAULT, AlertLevel::CAUTION,
                 "N1 SENSOR FAULT - LEFT", data.timestamp);
    }

    // 右发N1传感器故障
    if (!data.rightEngine.n1SensorValid)
    {
        addAlert(FaultType::SENSOR_FAULT, AlertLevel::CAUTION,
                 "N1 SENSOR FAULT - RIGHT", data.timestamp);
    }

    // 左发EGT传感器故障
    if (!data.leftEngine.egtSensorValid)
    {
        addAlert(FaultType::SENSOR_FAULT, AlertLevel::CAUTION,
                 "EGT SENSOR FAULT - LEFT", data.timestamp);
    }

    // 右发EGT传感器故障
    if (!data.rightEngine.egtSensorValid)
    {
        addAlert(FaultType::SENSOR_FAULT, AlertLevel::CAUTION,
                 "EGT SENSOR FAULT - RIGHT", data.timestamp);
    }

    // 单发双传感器失效（严重）
    if (leftFaults == 2)
    {
        addAlert(FaultType::SENSOR_FAULT, AlertLevel::WARNING,
                 "DUAL SENSOR FAULT - LEFT ENGINE", data.timestamp);
    }
    if (rightFaults == 2)
    {
        addAlert(FaultType::SENSOR_FAULT, AlertLevel::WARNING,
                 "DUAL SENSOR FAULT - RIGHT ENGINE", data.timestamp);
    }
}

void AlertManager::checkFuelAbnormal(const SystemData &data)
{
    // 迟滞阈值定义
    const double FUEL_QTY_HYSTERESIS = 50.0;  // 50kg
    const double FUEL_FLOW_HYSTERESIS = 2.0;  // 2kg/min
    const double IMBALANCE_HYSTERESIS = 0.02; // 2%

    // 1. 燃油传感器故障
    if (!data.fuelData.fuelSensorValid)
    {
        addAlert(FaultType::SENSOR_FAULT, AlertLevel::CAUTION,
                 "FUEL SENSOR FAULT", data.timestamp);
    }

    // 2. 燃油余量低（< 1000kg）
    // 注意：使用capacity而不是fuelQuantity，因为EngineSimulator主要更新capacity
    if (data.fuelData.fuelSensorValid)
    {
        // < 500 (WARNING)
        bool isCritical = isAlertActive(FaultType::FUEL_FLOW_LOW, "FUEL QUANTITY CRITICAL");
        double thresholdCritical = isCritical ? (500.0 + FUEL_QTY_HYSTERESIS) : 500.0;

        if (data.fuelData.capacity < thresholdCritical)
        {
            addAlert(FaultType::FUEL_FLOW_LOW, AlertLevel::WARNING,
                     "FUEL QUANTITY CRITICAL", data.timestamp);
        }
        else
        {
            // < 1000 (CAUTION)
            bool isLow = isAlertActive(FaultType::FUEL_FLOW_LOW, "FUEL QUANTITY LOW");
            double thresholdLow = isLow ? (1000.0 + FUEL_QTY_HYSTERESIS) : 1000.0;

            if (data.fuelData.capacity < thresholdLow)
            {
                addAlert(FaultType::FUEL_FLOW_LOW, AlertLevel::CAUTION,
                         "FUEL QUANTITY LOW", data.timestamp);
            }
        }
    }

    // 3. 左发燃油流量超限（> 50 kg/min）
    bool isLeftHigh = isAlertActive(FaultType::FUEL_FLOW_HIGH, "FUEL FLOW HIGH - LEFT");
    double thresholdLeft = isLeftHigh ? (50.0 - FUEL_FLOW_HYSTERESIS) : 50.0;

    if (data.leftEngine.fuelFlow > thresholdLeft)
    {
        addAlert(FaultType::FUEL_FLOW_HIGH, AlertLevel::WARNING,
                 "FUEL FLOW HIGH - LEFT", data.timestamp);
    }

    // 右发燃油流量超限
    bool isRightHigh = isAlertActive(FaultType::FUEL_FLOW_HIGH, "FUEL FLOW HIGH - RIGHT");
    double thresholdRight = isRightHigh ? (50.0 - FUEL_FLOW_HYSTERESIS) : 50.0;

    if (data.rightEngine.fuelFlow > thresholdRight)
    {
        addAlert(FaultType::FUEL_FLOW_HIGH, AlertLevel::WARNING,
                 "FUEL FLOW HIGH - RIGHT", data.timestamp);
    }

    // 4. 燃油不平衡（双发流量差异>15%）
    double avgFlow = (data.leftEngine.fuelFlow + data.rightEngine.fuelFlow) / 2.0;
    if (avgFlow > 5.0)
    { // 只在有显著流量时检测
        double leftDiff = std::abs(data.leftEngine.fuelFlow - avgFlow) / avgFlow;
        double rightDiff = std::abs(data.rightEngine.fuelFlow - avgFlow) / avgFlow;

        bool isImbalance = isAlertActive(FaultType::FUEL_IMBALANCE, "FUEL IMBALANCE");
        double thresholdImbalance = isImbalance ? (0.15 - IMBALANCE_HYSTERESIS) : 0.15;

        if (leftDiff > thresholdImbalance || rightDiff > thresholdImbalance)
        {
            addAlert(FaultType::FUEL_IMBALANCE, AlertLevel::CAUTION,
                     "FUEL IMBALANCE", data.timestamp);
        }
    }
}

bool AlertManager::isAlertActive(FaultType faultType, const std::string &message) const
{
    for (const auto &alert : activeAlerts_)
    {
        if (alert.faultType == faultType)
        {
            if (message.empty() || alert.message == message)
            {
                return true;
            }
        }
    }
    return false;
}

void AlertManager::checkThrustAbnormal(const SystemData &data)
{
    // 迟滞阈值定义
    const double N1_HYSTERESIS = 2.0; // 2% 的迟滞区间

    // 检查左发转速
    if (data.leftEngine.n1SensorValid)
    {
        // N1 > 120% (DANGER)
        bool isOver120 = isAlertActive(FaultType::N1_OVERSPEED, "N1 OVERSPEED CRITICAL - LEFT");
        double threshold120 = isOver120 ? (120.0 - N1_HYSTERESIS) : 120.0;

        if (data.leftEngine.n1Percentage > threshold120)
        {
            addAlert(FaultType::N1_OVERSPEED, AlertLevel::DANGER,
                     "N1 OVERSPEED CRITICAL - LEFT", data.timestamp);
        }
        else
        {
            // N1 > 105% (WARNING)
            bool isOver105 = isAlertActive(FaultType::N1_OVERSPEED, "N1 OVERSPEED - LEFT");
            double threshold105 = isOver105 ? (105.0 - N1_HYSTERESIS) : 105.0;

            if (data.leftEngine.n1Percentage > threshold105)
            {
                addAlert(FaultType::N1_OVERSPEED, AlertLevel::WARNING,
                         "N1 OVERSPEED - LEFT", data.timestamp);
            }
        }

        // N1 < 30% (CAUTION)
        if (data.leftEngine.state == SystemState::RUNNING)
        {
            bool isLow = isAlertActive(FaultType::N1_LOW, "N1 LOW - LEFT");
            double thresholdLow = isLow ? (30.0 + N1_HYSTERESIS) : 30.0;

            if (data.leftEngine.n1Percentage < thresholdLow)
            {
                addAlert(FaultType::N1_LOW, AlertLevel::CAUTION,
                         "N1 LOW - LEFT", data.timestamp);
            }
        }
    }

    // 检查右发转速
    if (data.rightEngine.n1SensorValid)
    {
        // N1 > 120% (DANGER)
        bool isOver120 = isAlertActive(FaultType::N1_OVERSPEED, "N1 OVERSPEED CRITICAL - RIGHT");
        double threshold120 = isOver120 ? (120.0 - N1_HYSTERESIS) : 120.0;

        if (data.rightEngine.n1Percentage > threshold120)
        {
            addAlert(FaultType::N1_OVERSPEED, AlertLevel::DANGER,
                     "N1 OVERSPEED CRITICAL - RIGHT", data.timestamp);
        }
        else
        {
            // N1 > 105% (WARNING)
            bool isOver105 = isAlertActive(FaultType::N1_OVERSPEED, "N1 OVERSPEED - RIGHT");
            double threshold105 = isOver105 ? (105.0 - N1_HYSTERESIS) : 105.0;

            if (data.rightEngine.n1Percentage > threshold105)
            {
                addAlert(FaultType::N1_OVERSPEED, AlertLevel::WARNING,
                         "N1 OVERSPEED - RIGHT", data.timestamp);
            }
        }

        // N1 < 30% (CAUTION)
        if (data.rightEngine.state == SystemState::RUNNING)
        {
            bool isLow = isAlertActive(FaultType::N1_LOW, "N1 LOW - RIGHT");
            double thresholdLow = isLow ? (30.0 + N1_HYSTERESIS) : 30.0;

            if (data.rightEngine.n1Percentage < thresholdLow)
            {
                addAlert(FaultType::N1_LOW, AlertLevel::CAUTION,
                         "N1 LOW - RIGHT", data.timestamp);
            }
        }
    }

    // 检查启动超时（启动超过60秒）
    if (isStartingPhase(data.leftEngine.state) &&
        data.leftEngine.startupTime > 60.0)
    {
        addAlert(FaultType::STARTER_TIMEOUT, AlertLevel::WARNING,
                 "STARTER TIMEOUT - LEFT", data.timestamp);
    }

    if (isStartingPhase(data.rightEngine.state) &&
        data.rightEngine.startupTime > 60.0)
    {
        addAlert(FaultType::STARTER_TIMEOUT, AlertLevel::WARNING,
                 "STARTER TIMEOUT - RIGHT", data.timestamp);
    }
}

void AlertManager::checkTempAbnormal(const SystemData &data)
{
    // 迟滞阈值定义
    const double EGT_HYSTERESIS = 15.0; // 15度迟滞

    // 左发温度检查
    if (data.leftEngine.egtSensorValid)
    {
        bool isStarting = isStartingPhase(data.leftEngine.state);
        double temp = data.leftEngine.egtTemperature;

        if (isStarting)
        {
            // 启动阶段：临时允许950°C
            // > 1000 (DANGER)
            bool isOver1000 = isAlertActive(FaultType::EGT_OVERHEAT, "EGT OVERTEMP CRITICAL - LEFT");
            double threshold1000 = isOver1000 ? (1000.0 - EGT_HYSTERESIS) : 1000.0;

            if (temp > threshold1000)
            {
                // 超温2（红色，立即停车）
                addAlert(FaultType::EGT_OVERHEAT, AlertLevel::DANGER,
                         "EGT OVERTEMP CRITICAL - LEFT", data.timestamp);
            }
            else
            {
                // > 950 (WARNING)
                bool isOver950 = isAlertActive(FaultType::EGT_OVERHEAT, "EGT OVERTEMP - LEFT (STARTING)");
                double threshold950 = isOver950 ? (950.0 - EGT_HYSTERESIS) : 950.0;

                if (temp > threshold950)
                {
                    // 超温1（黄色，警告）
                    addAlert(FaultType::EGT_OVERHEAT, AlertLevel::WARNING,
                             "EGT OVERTEMP - LEFT (STARTING)", data.timestamp);
                }
            }
        }
        else if (isRunningPhase(data.leftEngine.state))
        {
            // 运行阶段：最高850°C
            // > 900 (DANGER)
            bool isOver900 = isAlertActive(FaultType::EGT_OVERHEAT, "EGT OVERTEMP CRITICAL - LEFT");
            double threshold900 = isOver900 ? (900.0 - EGT_HYSTERESIS) : 900.0;

            if (temp > threshold900)
            {
                // 超温4（红色）
                addAlert(FaultType::EGT_OVERHEAT, AlertLevel::DANGER,
                         "EGT OVERTEMP CRITICAL - LEFT", data.timestamp);
            }
            else
            {
                // > 850 (WARNING)
                bool isOver850 = isAlertActive(FaultType::EGT_OVERHEAT, "EGT OVERTEMP - LEFT");
                double threshold850 = isOver850 ? (850.0 - EGT_HYSTERESIS) : 850.0;

                if (temp > threshold850)
                {
                    // 超温3（黄色）
                    addAlert(FaultType::EGT_OVERHEAT, AlertLevel::WARNING,
                             "EGT OVERTEMP - LEFT", data.timestamp);
                }
                else
                {
                    // < 400 (CAUTION)
                    bool isLow = isAlertActive(FaultType::EGT_LOW, "EGT LOW - LEFT");
                    double thresholdLow = isLow ? (400.0 + EGT_HYSTERESIS) : 400.0;

                    if (temp < thresholdLow)
                    {
                        // 温度过低
                        addAlert(FaultType::EGT_LOW, AlertLevel::CAUTION,
                                 "EGT LOW - LEFT", data.timestamp);
                    }
                }
            }
        }
    }

    // 右发温度检查
    if (data.rightEngine.egtSensorValid)
    {
        bool isStarting = isStartingPhase(data.rightEngine.state);
        double temp = data.rightEngine.egtTemperature;

        if (isStarting)
        {
            // > 1000 (DANGER)
            bool isOver1000 = isAlertActive(FaultType::EGT_OVERHEAT, "EGT OVERTEMP CRITICAL - RIGHT");
            double threshold1000 = isOver1000 ? (1000.0 - EGT_HYSTERESIS) : 1000.0;

            if (temp > threshold1000)
            {
                addAlert(FaultType::EGT_OVERHEAT, AlertLevel::DANGER,
                         "EGT OVERTEMP CRITICAL - RIGHT", data.timestamp);
            }
            else
            {
                // > 950 (WARNING)
                bool isOver950 = isAlertActive(FaultType::EGT_OVERHEAT, "EGT OVERTEMP - RIGHT (STARTING)");
                double threshold950 = isOver950 ? (950.0 - EGT_HYSTERESIS) : 950.0;

                if (temp > threshold950)
                {
                    addAlert(FaultType::EGT_OVERHEAT, AlertLevel::WARNING,
                             "EGT OVERTEMP - RIGHT (STARTING)", data.timestamp);
                }
            }
        }
        else if (isRunningPhase(data.rightEngine.state))
        {
            // > 900 (DANGER)
            bool isOver900 = isAlertActive(FaultType::EGT_OVERHEAT, "EGT OVERTEMP CRITICAL - RIGHT");
            double threshold900 = isOver900 ? (900.0 - EGT_HYSTERESIS) : 900.0;

            if (temp > threshold900)
            {
                addAlert(FaultType::EGT_OVERHEAT, AlertLevel::DANGER,
                         "EGT OVERTEMP CRITICAL - RIGHT", data.timestamp);
            }
            else
            {
                // > 850 (WARNING)
                bool isOver850 = isAlertActive(FaultType::EGT_OVERHEAT, "EGT OVERTEMP - RIGHT");
                double threshold850 = isOver850 ? (850.0 - EGT_HYSTERESIS) : 850.0;

                if (temp > threshold850)
                {
                    addAlert(FaultType::EGT_OVERHEAT, AlertLevel::WARNING,
                             "EGT OVERTEMP - RIGHT", data.timestamp);
                }
                else
                {
                    // < 400 (CAUTION)
                    bool isLow = isAlertActive(FaultType::EGT_LOW, "EGT LOW - RIGHT");
                    double thresholdLow = isLow ? (400.0 + EGT_HYSTERESIS) : 400.0;

                    if (temp < thresholdLow)
                    {
                        addAlert(FaultType::EGT_LOW, AlertLevel::CAUTION,
                                 "EGT LOW - RIGHT", data.timestamp);
                    }
                }
            }
        }
    }
}

// ==================== 私有辅助函数 ====================

void AlertManager::addAlert(FaultType faultType, AlertLevel level,
                            const std::string &message, double timestamp)
{
    // 1. 检查告警是否已存在（避免重复）
    bool exists = false;
    for (auto &alert : activeAlerts_)
    {
        if (alert.faultType == faultType && alert.message == message)
        {
            // 如果已存在，刷新displayTimer并标记为已更新
            alert.displayTimer = 5.0;
            alert.isActive = true;
            alert.updated = true;
            exists = true;
            break;
        }
    }

    // 2. 如果不存在，创建新告警
    if (!exists)
    {
        AlertInfo newAlert;
        newAlert.faultType = faultType;
        newAlert.level = level;
        newAlert.message = message;
        newAlert.timestamp = timestamp;
        newAlert.isActive = true;
        newAlert.updated = true;
        newAlert.displayTimer = 5.0; // 5秒显示时间

        activeAlerts_.push_back(newAlert);

        // 3. 添加到newAlerts_（用于日志记录）
        if (shouldLogAlert(faultType, timestamp))
        {
            newAlerts_.push_back(newAlert);
            lastLogTime_[faultType] = timestamp;
        }
    }
}

bool AlertManager::alertExists(FaultType faultType) const
{
    // TODO: 实现告警存在性检查
    // 遍历activeAlerts_，查找指定faultType
    return false; // 占位返回
}

void AlertManager::removeAlert(FaultType faultType)
{
    // TODO: 实现移除告警
    // 使用erase-remove惯用法从activeAlerts_中移除
}

bool AlertManager::shouldLogAlert(FaultType faultType, double currentTime)
{
    // 1. 检查是否首次出现该故障
    auto it = lastLogTime_.find(faultType);
    if (it == lastLogTime_.end())
    {
        return true; // 首次出现，应该记录
    }

    // 2. 检查距离上次记录是否超过5秒
    double timeSinceLastLog = currentTime - it->second;
    return timeSinceLastLog >= 5.0;
}

void AlertManager::updateHighestLevel()
{
    highestLevel_ = AlertLevel::NORMAL;

    // 遍历所有活跃告警，找出最高级别
    for (const auto &alert : activeAlerts_)
    {
        if (alert.isActive)
        {
            if (alert.level > highestLevel_)
            {
                highestLevel_ = alert.level;
            }
        }
    }
}

bool AlertManager::isStartingPhase(SystemState state) const
{
    return state == SystemState::STARTING_P1 ||
           state == SystemState::STARTING_P2;
}

bool AlertManager::isRunningPhase(SystemState state) const
{
    return state == SystemState::RUNNING;
}

void AlertManager::checkEngineSensors(const EngineData &engine, EngineID engineID)
{
    // TODO: 实现单个发动机传感器检查
    // 1. 检查N1传感器状态
    // 2. 检查EGT传感器状态
    // 3. 根据失效情况分类（单传感器/单发双传感器）
}

std::string AlertManager::generateAlertMessage(FaultType faultType,
                                               EngineID engineID) const
{
    // TODO: 实现告警消息生成
    // 根据faultType返回相应的英文消息
    // 例如：
    // - "N1 SENSOR FAULT - LEFT ENGINE"
    // - "FUEL LOW"
    // - "OVERSPEED - WARNING"
    // - "EGT OVERTEMP - CAUTION"

    return "ALERT MESSAGE"; // 占位返回
}
