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
    // TODO: 实现核心检测逻辑
    // 1. 清空newAlerts_列表
    // 2. 调用各个子检测函数
    // 3. 更新最高告警级别
    // 4. 返回最高级别

    return AlertLevel::NORMAL; // 占位返回
}

void AlertManager::updateTimers(double dt)
{
    // TODO: 实现计时器更新
    // 1. 遍历activeAlerts_
    // 2. 更新每个告警的displayTimer
    // 3. 如果displayTimer <= 0，设置isActive = false
    // 4. 清理不活跃的告警
}

// ==================== 数据访问接口 ====================

std::vector<std::string> AlertManager::getActiveMessages() const
{
    std::vector<std::string> messages;
    // TODO: 实现获取活跃消息
    // 遍历activeAlerts_，只返回displayTimer > 0的消息
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
    // TODO: 实现传感器故障检测（6种）
    // 1. 检查左发动机N1传感器
    // 2. 检查右发动机N1传感器
    // 3. 检查左发动机EGT传感器
    // 4. 检查右发动机EGT传感器
    // 5. 判断单传感器故障 vs 单发双传感器故障 vs 双发故障
}

void AlertManager::checkFuelAbnormal(const SystemData &data)
{
    // TODO: 实现燃油异常检测（3种）
    // 1. 燃油余量低（< 1000）
    // 2. 燃油传感器故障（sensorValid == false）
    // 3. 燃油流速超限（> 50）
}

void AlertManager::checkThrustAbnormal(const SystemData &data)
{
    // TODO: 实现转速异常检测（2种）
    // 1. 超转1（N1 > 105%）
    // 2. 超转2（N1 > 120%，需要触发停车）
}

void AlertManager::checkTempAbnormal(const SystemData &data)
{
    // TODO: 实现温度异常检测（4种）
    // 1. 判断当前是启动阶段还是稳态阶段
    // 2. 根据阶段应用不同的温度阈值
    // 3. 超温1/3（琥珀色）
    // 4. 超温2/4（红色，触发停车）
}

// ==================== 私有辅助函数 ====================

void AlertManager::addAlert(FaultType faultType, AlertLevel level,
                            const std::string &message, double timestamp)
{
    // TODO: 实现添加告警
    // 1. 检查告警是否已存在
    // 2. 如果不存在，创建新的AlertInfo
    // 3. 设置displayTimer = 5.0秒
    // 4. 添加到activeAlerts_和newAlerts_
    // 5. 更新lastLogTime_
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
    // TODO: 实现日志记录判断
    // 1. 检查lastLogTime_中是否有该故障类型
    // 2. 如果没有，或者距离上次记录超过5秒，返回true
    // 3. 否则返回false
    return true; // 占位返回
}

void AlertManager::updateHighestLevel()
{
    // TODO: 实现更新最高告警级别
    // 遍历activeAlerts_，找出最高级别
    highestLevel_ = AlertLevel::NORMAL;
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
