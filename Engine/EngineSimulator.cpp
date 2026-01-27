#include "EngineSimulator.h"
#include <cmath>
#include <algorithm>

// ==================== 构造与析构 ====================

EngineSimulator::EngineSimulator()
    : startingTimer_(0.0),
      stoppingTimer_(0.0),
      thrustLevel_(1.0),
      targetLeftN1_(0.0),
      targetLeftEGT_(Constants::T0_AMBIENT),
      targetRightN1_(0.0),
      targetRightEGT_(Constants::T0_AMBIENT),
      randomGenerator_(std::random_device{}()),
      fluctuationDist_(-1.0, 1.0),
      currentFaultType_(FaultType::NONE),
      currentFaultEngineID_(EngineID::LEFT)
{
    // 初始化系统数据
    systemData_.leftEngine.state = SystemState::OFF;
    systemData_.rightEngine.state = SystemState::OFF;
    systemData_.elapsedTime = 0.0;
}

EngineSimulator::~EngineSimulator()
{
    // 清理资源
}

// ==================== 核心更新函数 ====================

void EngineSimulator::update(double dt)
{
    // 1. 更新运行时间
    systemData_.elapsedTime += dt;
    systemData_.timestamp = systemData_.elapsedTime;

    // 2. 根据当前状态调用相应的更新函数（同步两台发动机）
    SystemState currentState = systemData_.leftEngine.state;
    systemData_.systemState = currentState;

    switch (currentState)
    {
    case SystemState::OFF:
        // 什么都不做
        break;

    case SystemState::STARTING_P1:
        updateStartingPhase1(dt);
        break;

    case SystemState::STARTING_P2:
        updateStartingPhase2(dt);
        break;

    case SystemState::RUNNING:
        updateRunningPhase(dt);
        break;

    case SystemState::STOPPING:
        updateStoppingPhase(dt);
        break;
    }

    // 3. 更新燃油消耗
    updateFuelConsumption(dt);

    // 4. 更新传感器读数
    updateSensorReadings(systemData_.leftEngine);
    updateSensorReadings(systemData_.rightEngine);

    // 5. 同步两台发动机和燃油数据
    synchronizeEngines();
    systemData_.fuelData = systemData_.fuel;
}

// ==================== 控制接口函数 ====================

void EngineSimulator::startEngine()
{
    // 1. 检查当前状态是否为OFF或STOPPING（允许在停车过程中重新启动）
    if (systemData_.leftEngine.state != SystemState::OFF &&
        systemData_.leftEngine.state != SystemState::STOPPING)
    {
        return; // 已经启动或正在运行，不重复启动
    }

    // 2. 切换到STARTING_P1状态
    systemData_.leftEngine.state = SystemState::STARTING_P1;
    systemData_.rightEngine.state = SystemState::STARTING_P1;

    // 3. 重置启动计时器
    startingTimer_ = 0.0;

    // 4. 初始化相关参数
    // 如果是从STOPPING状态重启，可以选择保留当前转速，这里简单处理为重置
    systemData_.leftEngine.n1Percentage = 0.0;
    systemData_.leftEngine.egtTemperature = Constants::T0_AMBIENT;
    systemData_.rightEngine.n1Percentage = 0.0;
    systemData_.rightEngine.egtTemperature = Constants::T0_AMBIENT;
    systemData_.fuel.flowRate = 0.0;
}

void EngineSimulator::stopEngine()
{
    // 最高优先级，无论当前状态如何都执行

    // 1. 立即切换到STOPPING状态
    systemData_.leftEngine.state = SystemState::STOPPING;
    systemData_.rightEngine.state = SystemState::STOPPING;

    // 2. 燃油流速立即归零
    systemData_.fuel.flowRate = 0.0;

    // 3. 重置停车计时器
    stoppingTimer_ = 0.0;
}

void EngineSimulator::adjustThrust(int direction)
{
    // 只在运行状态才能调整推力
    if (systemData_.leftEngine.state != SystemState::RUNNING)
    {
        return;
    }

    // 1. 调整推力级别
    thrustLevel_ += direction * 0.02;             // 每次调整2%
    thrustLevel_ = clamp(thrustLevel_, 0.5, 1.5); // 限制在50%-150%

    // 2. 移除手动调整N1/EGT的代码，让updateRunningPhase通过thrustLevel_自动平滑过渡
    // 这样可以避免"先升后降"的抖动问题
}

void EngineSimulator::injectFault(EngineID engineID, FaultType faultType)
{
    // 1. 先清除之前的故障状态（确保每次只测试一种故障）
    clearFault(EngineID::LEFT);
    clearFault(EngineID::RIGHT);

    // 2. 保存当前故障状态
    currentFaultType_ = faultType;
    currentFaultEngineID_ = engineID;

    // 根据engineID选择目标发动机
    EngineData *targetEngine = (engineID == EngineID::LEFT) ? &systemData_.leftEngine : &systemData_.rightEngine;

    // 根据faultType设置对应传感器的valid标志为false
    switch (faultType)
    {
    case FaultType::SENSOR_FAULT:
        // 随机让一个传感器失效
        if (fluctuationDist_(randomGenerator_) > 0)
        {
            targetEngine->n1Sensors.valid1 = false;
        }
        else
        {
            targetEngine->egtSensors.valid1 = false;
        }
        break;

    case FaultType::SINGLE_N1_SENSOR_FAULT:
        targetEngine->n1Sensors.valid1 = false;
        break;

    case FaultType::SINGLE_ENGINE_N1_FAULT:
        // 单发双传感器失效
        targetEngine->n1Sensors.valid1 = false;
        targetEngine->n1Sensors.valid2 = false;
        targetEngine->n1SensorValid = false; // 标记整体无效
        break;

    case FaultType::SINGLE_EGT_SENSOR_FAULT:
        targetEngine->egtSensors.valid1 = false;
        break;

    case FaultType::SINGLE_ENGINE_EGT_FAULT:
        // 单发双传感器失效
        targetEngine->egtSensors.valid1 = false;
        targetEngine->egtSensors.valid2 = false;
        targetEngine->egtSensorValid = false; // 标记整体无效
        break;

    case FaultType::DUAL_ENGINE_SENSOR_FAULT:
        // 双发传感器故障
        systemData_.leftEngine.n1SensorValid = false;

        systemData_.rightEngine.n1Sensors.valid1 = false;
        systemData_.rightEngine.n1Sensors.valid2 = false;
        systemData_.rightEngine.n1SensorValid = false;
        break;

    case FaultType::FUEL_SENSOR_FAULT:
        systemData_.fuel.fuelSensorValid = false;
        break;

    // 其他数值型故障在updateRunningPhase中持续应用
    default:
        break;
    }

    // 更新便捷访问字段
    targetEngine->n1SensorValid = targetEngine->n1Sensors.valid1 || targetEngine->n1Sensors.valid2;
    targetEngine->egtSensorValid = targetEngine->egtSensors.valid1 || targetEngine->egtSensors.valid2;
}

void EngineSimulator::clearFault(EngineID engineID)
{
    // 重置故障类型
    currentFaultType_ = FaultType::NONE;

    // 恢复所有发动机的传感器
    EngineData *engines[] = {&systemData_.leftEngine, &systemData_.rightEngine};

    for (auto *engine : engines)
    {
        engine->n1Sensors.valid1 = true;
        engine->n1Sensors.valid2 = true;
        engine->n1SensorValid = true;

        engine->egtSensors.valid1 = true;
        engine->egtSensors.valid2 = true;
        engine->egtSensorValid = true;
    }

    // 恢复燃油系统
    systemData_.fuel.fuelSensorValid = true;
    if (systemData_.fuel.capacity < 1000.0)
    {
        systemData_.fuel.capacity = 5000.0; // 恢复到正常油量
    }

    // 燃油流速不需要强制重置，让updateRunningPhase平滑过渡回正常值
}

// ==================== 数据访问接口 ====================

SystemData EngineSimulator::getLatestData() const
{
    return systemData_;
}

EngineData EngineSimulator::getEngineData(EngineID engineID) const
{
    if (engineID == EngineID::LEFT)
    {
        return systemData_.leftEngine;
    }
    else
    {
        return systemData_.rightEngine;
    }
}

FuelData EngineSimulator::getFuelData() const
{
    return systemData_.fuel;
}

double EngineSimulator::getElapsedTime() const
{
    return systemData_.elapsedTime;
}

// ==================== 状态查询接口 ====================

bool EngineSimulator::isRunning() const
{
    return systemData_.leftEngine.state == SystemState::RUNNING ||
           systemData_.rightEngine.state == SystemState::RUNNING;
}

bool EngineSimulator::isStarting() const
{
    return systemData_.leftEngine.state == SystemState::STARTING_P1 ||
           systemData_.leftEngine.state == SystemState::STARTING_P2 ||
           systemData_.rightEngine.state == SystemState::STARTING_P1 ||
           systemData_.rightEngine.state == SystemState::STARTING_P2;
}

bool EngineSimulator::isStopping() const
{
    return systemData_.leftEngine.state == SystemState::STOPPING ||
           systemData_.rightEngine.state == SystemState::STOPPING;
}

bool EngineSimulator::isFaultActive() const
{
    return currentFaultType_ != FaultType::NONE;
}

bool EngineSimulator::isFaultTargetReached() const
{
    if (currentFaultType_ == FaultType::NONE)
    {
        return true;
    }

    // 定义判断"到达"的容差
    // 由于有随机波动(±3%)，容差必须大于波动范围
    // 但不能太大，否则会导致刚进入告警区就误判为"已到达"从而提前停车
    // N1: 3%波动 -> 容差设为 2.0 (需要更精确的接近)
    // EGT: 3%波动 -> 容差设为 15.0 (需要更精确的接近)
    const double TOLERANCE_N1 = 2.0;
    const double TOLERANCE_EGT = 15.0;

    bool leftReached = true;
    bool rightReached = true;

    // 检查左发
    if (std::abs(systemData_.leftEngine.n1Percentage - targetLeftN1_) > TOLERANCE_N1)
        leftReached = false;
    if (std::abs(systemData_.leftEngine.egtTemperature - targetLeftEGT_) > TOLERANCE_EGT)
        leftReached = false;

    // 检查右发
    if (std::abs(systemData_.rightEngine.n1Percentage - targetRightN1_) > TOLERANCE_N1)
        rightReached = false;
    if (std::abs(systemData_.rightEngine.egtTemperature - targetRightEGT_) > TOLERANCE_EGT)
        rightReached = false;

    // 只要有一个发动机未达到目标，就认为整体未达到
    // (对于单发故障，未故障的发动机始终处于"到达"状态，所以逻辑成立)
    return leftReached && rightReached;
}

// ==================== 私有辅助函数 ====================

// 辅助函数：平滑移动数值
double moveTowards(double current, double target, double maxDelta)
{
    if (std::abs(target - current) <= maxDelta)
    {
        return target;
    }
    return current + (target > current ? maxDelta : -maxDelta);
}

void EngineSimulator::updateStartingPhase1(double dt)
{
    startingTimer_ += dt;

    // 线性增长：N1以每秒10^4转速度增加（换算成百分比）
    double n1Rate = (Constants::PHASE1_N1_RATE / Constants::RATED_RPM) * 100.0; // 转成百分比/秒
    systemData_.leftEngine.n1Percentage += n1Rate * dt;
    systemData_.rightEngine.n1Percentage += n1Rate * dt;

    // 燃油流速以每秒5单位速度增加
    systemData_.fuel.flowRate += Constants::PHASE1_FUEL_RATE * dt;

    // 温度保持在室温
    systemData_.leftEngine.egtTemperature = Constants::T0_AMBIENT;
    systemData_.rightEngine.egtTemperature = Constants::T0_AMBIENT;

    // 持续2秒后切换到STARTING_P2
    if (startingTimer_ >= Constants::PHASE1_DURATION)
    {
        systemData_.leftEngine.state = SystemState::STARTING_P2;
        systemData_.rightEngine.state = SystemState::STARTING_P2;
        startingTimer_ = 0.0; // 重置计时器用于阶段2
    }
}

void EngineSimulator::updateStartingPhase2(double dt)
{
    startingTimer_ += dt;

    // 使用对数函数计算（t从2秒开始）
    double t = Constants::PHASE1_DURATION + startingTimer_;

    // 计算目标值
    double targetN1 = calculateN1Logarithmic(t);
    double targetEGT = calculateEGTLogarithmic(t);
    double targetFlow = calculateFuelFlowLogarithmic(t);

    // 启动阶段保持平滑增长，不添加波动
    // targetN1 = addFluctuation(targetN1, 0.01);
    // targetEGT = addFluctuation(targetEGT, 0.01);
    // targetFlow = addFluctuation(targetFlow, 0.01);

    // 更新数值
    systemData_.leftEngine.n1Percentage = targetN1;
    systemData_.leftEngine.egtTemperature = targetEGT;
    systemData_.rightEngine.n1Percentage = targetN1;
    systemData_.rightEngine.egtTemperature = targetEGT;
    systemData_.fuel.flowRate = targetFlow;

    // 应用启动阶段故障
    EngineData *targetEngine = (currentFaultEngineID_ == EngineID::LEFT) ? &systemData_.leftEngine : &systemData_.rightEngine;

    if (currentFaultType_ == FaultType::OVERTEMP_1_STARTING)
    {
        targetEngine->egtTemperature = 980.0; // > 950
    }
    else if (currentFaultType_ == FaultType::OVERTEMP_2_STARTING)
    {
        targetEngine->egtTemperature = 1050.0; // > 1000
    }

    // 达到95%额定转速后切换到RUNNING
    if (systemData_.leftEngine.n1Percentage >= Constants::N1_STABLE_THRESHOLD)
    {
        systemData_.leftEngine.state = SystemState::RUNNING;
        systemData_.rightEngine.state = SystemState::RUNNING;
        // 保持当前的燃油流速，不强制重置
    }
}

void EngineSimulator::updateRunningPhase(double dt)
{
    // 1. 计算基础目标值 (基于推力等级)
    // 假设 thrustLevel_ = 1.0 时: N1=95%, EGT=850, Flow=45
    double targetN1 = 95.0 * thrustLevel_;
    double targetEGT = 850.0 * thrustLevel_;
    double targetFlow = 45.0 * thrustLevel_;

    // 2. 根据故障类型覆盖目标值
    // 默认两台发动机目标一致
    double leftTargetN1 = targetN1;
    double leftTargetEGT = targetEGT;
    double rightTargetN1 = targetN1;
    double rightTargetEGT = targetEGT;
    double finalTargetFlow = targetFlow;

    switch (currentFaultType_)
    {
    case FaultType::FUEL_FLOW_EXCEED:
    case FaultType::FUEL_FLOW_HIGH:
        finalTargetFlow = 55.0;
        // 联动: 稍微增加转速和温度
        if (currentFaultEngineID_ == EngineID::LEFT)
        {
            leftTargetN1 = 102.0;
            leftTargetEGT = 900.0;
        }
        else
        {
            rightTargetN1 = 102.0;
            rightTargetEGT = 900.0;
        }
        break;

    case FaultType::FUEL_LOW:
        systemData_.fuel.capacity = 800.0; // 强制设置为低油量
        break;

    case FaultType::OVERSPEED_1:
        if (currentFaultEngineID_ == EngineID::LEFT)
        {
            leftTargetN1 = 108.0;
            leftTargetEGT = 920.0;
        }
        else
        {
            rightTargetN1 = 108.0;
            rightTargetEGT = 920.0;
        }
        finalTargetFlow = 48.0;
        break;

    case FaultType::OVERSPEED_2:
        if (currentFaultEngineID_ == EngineID::LEFT)
        {
            leftTargetN1 = 126.0; // 提高目标值，确保超过120%后仍有上升空间
            leftTargetEGT = 960.0;
        }
        else
        {
            rightTargetN1 = 126.0;
            rightTargetEGT = 960.0;
        }
        finalTargetFlow = 52.0;
        break;

    case FaultType::OVERTEMP_3_RUNNING:
        if (currentFaultEngineID_ == EngineID::LEFT)
        {
            leftTargetEGT = 980.0; // > 950 (WARNING)
            leftTargetN1 = 98.0;
        }
        else
        {
            rightTargetEGT = 980.0;
            rightTargetN1 = 98.0;
        }
        finalTargetFlow = 46.0;
        break;

    case FaultType::OVERTEMP_4_RUNNING:
        if (currentFaultEngineID_ == EngineID::LEFT)
        {
            leftTargetEGT = 1080.0; // 提高目标值，确保超过1000度后仍有上升空间
            leftTargetN1 = 100.0;
        }
        else
        {
            rightTargetEGT = 1080.0;
            rightTargetN1 = 100.0;
        }
        finalTargetFlow = 48.0;
        break;

    case FaultType::OVERTEMP_1_STARTING:
        // 即使在运行阶段触发，也模拟超温效果
        if (currentFaultEngineID_ == EngineID::LEFT)
        {
            leftTargetEGT = 900.0; // > 850
        }
        else
        {
            rightTargetEGT = 900.0;
        }
        break;

    case FaultType::OVERTEMP_2_STARTING:
        if (currentFaultEngineID_ == EngineID::LEFT)
        {
            leftTargetEGT = 1080.0; // 提高目标值，确保超过1000度后仍有上升空间
        }
        else
        {
            rightTargetEGT = 1080.0;
        }
        break;

    default:
        break;
    }

    // 3. 平滑过渡 (Smooth Transition)
    // 定义变化率 (每秒变化量)
    double n1Rate = 20.0;   // % per second
    double egtRate = 100.0; // C per second
    double flowRate = 10.0; // units per second

    // 更新成员变量中的目标值（用于 isFaultTargetReached 判断）
    targetLeftN1_ = leftTargetN1;
    targetLeftEGT_ = leftTargetEGT;
    targetRightN1_ = rightTargetN1;
    targetRightEGT_ = rightTargetEGT;

    // 4. 应用随机波动 (Fluctuation)
    // 修正逻辑：将波动应用在"目标值"上，而不是"当前值"上。
    // 这样 moveTowards 会让指针去追逐一个在 ±3% 范围内跳动的目标，
    // 既实现了波动，又保证了数值永远不会漂移出这个范围，且移动是平滑的。

    double noisyLeftN1 = addFluctuation(leftTargetN1, Constants::FLUCTUATION_RANGE);
    double noisyLeftEGT = addFluctuation(leftTargetEGT, Constants::FLUCTUATION_RANGE);
    double noisyRightN1 = addFluctuation(rightTargetN1, Constants::FLUCTUATION_RANGE);
    double noisyRightEGT = addFluctuation(rightTargetEGT, Constants::FLUCTUATION_RANGE);
    double noisyFlow = addFluctuation(finalTargetFlow, Constants::FLUCTUATION_RANGE);

    // 执行平滑移动
    systemData_.leftEngine.n1Percentage = moveTowards(systemData_.leftEngine.n1Percentage, noisyLeftN1, n1Rate * dt);
    systemData_.leftEngine.egtTemperature = moveTowards(systemData_.leftEngine.egtTemperature, noisyLeftEGT, egtRate * dt);

    systemData_.rightEngine.n1Percentage = moveTowards(systemData_.rightEngine.n1Percentage, noisyRightN1, n1Rate * dt);
    systemData_.rightEngine.egtTemperature = moveTowards(systemData_.rightEngine.egtTemperature, noisyRightEGT, egtRate * dt);

    systemData_.fuel.flowRate = moveTowards(systemData_.fuel.flowRate, noisyFlow, flowRate * dt);

    // 限制最小值和最大值
    systemData_.fuel.flowRate = std::max(0.0, systemData_.fuel.flowRate);
    systemData_.leftEngine.n1Percentage = clamp(systemData_.leftEngine.n1Percentage, 0.0, 125.0);
    systemData_.rightEngine.n1Percentage = clamp(systemData_.rightEngine.n1Percentage, 0.0, 125.0);

    // 5. 更新单发燃油流速数据 (用于告警检测)
    // 假设总流速平均分配给两台发动机 (或者每台发动机流速等于总流速，取决于定义)
    // 根据AlertManager的逻辑 (>50报警)，这里的flowRate (45-55) 应该是单发流速
    systemData_.leftEngine.fuelFlow = systemData_.fuel.flowRate;
    systemData_.rightEngine.fuelFlow = systemData_.fuel.flowRate;
}

void EngineSimulator::updateStoppingPhase(double dt)
{
    stoppingTimer_ += dt;

    // 燃油流速保持为0
    systemData_.fuel.flowRate = 0.0;

    // 10秒内对数下降至0
    if (stoppingTimer_ >= Constants::STOPPING_DURATION)
    {
        // 完全停止
        systemData_.leftEngine.n1Percentage = 0.0;
        systemData_.leftEngine.egtTemperature = Constants::T0_AMBIENT;
        systemData_.rightEngine.n1Percentage = 0.0;
        systemData_.rightEngine.egtTemperature = Constants::T0_AMBIENT;
        systemData_.leftEngine.state = SystemState::OFF;
        systemData_.rightEngine.state = SystemState::OFF;
    }
    else
    {
        // 对数下降（使用指数函数模拟，底数<1）
        double progress = stoppingTimer_ / Constants::STOPPING_DURATION;
        double factor = std::pow(0.1, progress); // 从1降到0.1的指数曲线

        // 保存启动时的最大值用于计算
        static double maxN1 = systemData_.leftEngine.n1Percentage;
        static double maxEGT = systemData_.leftEngine.egtTemperature;

        if (stoppingTimer_ == dt)
        { // 刚开始停车
            maxN1 = systemData_.leftEngine.n1Percentage;
            maxEGT = systemData_.leftEngine.egtTemperature;
        }

        systemData_.leftEngine.n1Percentage = maxN1 * factor;
        systemData_.leftEngine.egtTemperature = Constants::T0_AMBIENT +
                                                (maxEGT - Constants::T0_AMBIENT) * factor;
        systemData_.rightEngine.n1Percentage = systemData_.leftEngine.n1Percentage;
        systemData_.rightEngine.egtTemperature = systemData_.leftEngine.egtTemperature;
    }
}

void EngineSimulator::updateFuelConsumption(double dt)
{
    // 计算公式：ΔC = -V * dt
    double consumption = systemData_.fuel.flowRate * dt;
    systemData_.fuel.capacity -= consumption;

    // 确保燃油余量不低于0
    systemData_.fuel.capacity = clamp(systemData_.fuel.capacity,
                                      Constants::FUEL_MIN,
                                      Constants::FUEL_MAX);

    // 如果燃油耗尽，强制停车
    if (systemData_.fuel.capacity <= 0.0 && isRunning())
    {
        // 切换到STOPPING状态
        systemData_.leftEngine.state = SystemState::STOPPING;
        systemData_.rightEngine.state = SystemState::STOPPING;
        stoppingTimer_ = 0.0;
    }
}

double EngineSimulator::calculateN1Logarithmic(double t)
{
    // N = 23000 * lg(t-1) + 20000
    // lg表示以10为底的对数
    // 转换成转速百分比：N / RATED_RPM * 100

    if (t <= 1.0)
        return 0.0; // 避免log(0)或负数

    double rpm = 23000.0 * std::log10(t - 1.0) + 20000.0;
    double percentage = (rpm / Constants::RATED_RPM) * 100.0;

    // 限制在有效范围内
    return clamp(percentage, 0.0, Constants::N1_MAX);
}

double EngineSimulator::calculateEGTLogarithmic(double t)
{
    // T = 900 * lg(t-1) + T0
    if (t <= 1.0)
        return Constants::T0_AMBIENT;

    double temp = 900.0 * std::log10(t - 1.0) + Constants::T0_AMBIENT;

    // 限制在有效范围内
    return clamp(temp, Constants::EGT_MIN, Constants::EGT_MAX);
}

double EngineSimulator::calculateFuelFlowLogarithmic(double t)
{
    // V = 42 * lg(t-1) + 10
    if (t <= 1.0)
        return 0.0;

    double flow = 42.0 * std::log10(t - 1.0) + 10.0;

    // 限制在有效范围内
    return clamp(flow, Constants::FUEL_FLOW_MIN, Constants::FUEL_FLOW_MAX);
}

double EngineSimulator::addFluctuation(double baseValue, double range)
{
    // 生成[-range, +range]范围内的随机百分比
    double randomPercent = fluctuationDist_(randomGenerator_) * range;
    return baseValue * (1.0 + randomPercent);
}

double EngineSimulator::clamp(double value, double minVal, double maxVal)
{
    return std::max(minVal, std::min(value, maxVal));
}

void EngineSimulator::updateSensorReadings(EngineData &engineData)
{
    // 将计算的实际值分配给双传感器
    // 添加微小的差异以增加真实性

    if (engineData.n1Sensors.valid1)
    {
        engineData.n1Sensors.value1 = addFluctuation(engineData.n1Percentage, 0.001);
    }
    if (engineData.n1Sensors.valid2)
    {
        engineData.n1Sensors.value2 = addFluctuation(engineData.n1Percentage, 0.001);
    }

    if (engineData.egtSensors.valid1)
    {
        engineData.egtSensors.value1 = addFluctuation(engineData.egtTemperature, 0.001);
    }
    if (engineData.egtSensors.valid2)
    {
        engineData.egtSensors.value2 = addFluctuation(engineData.egtTemperature, 0.001);
    }
}

void EngineSimulator::synchronizeEngines()
{
    // 确保左右发动机状态和数据基本一致
    // 可以添加微小差异以增加真实性
    systemData_.rightEngine.state = systemData_.leftEngine.state;

    // 数据已经在各阶段更新时同步，这里不需要额外操作
}
