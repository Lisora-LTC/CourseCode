#include "EngineSimulator.h"
#include <cmath>
#include <algorithm>

// ==================== 构造与析构 ====================

EngineSimulator::EngineSimulator()
    : startingTimer_(0.0),
      stoppingTimer_(0.0),
      thrustLevel_(1.0),
      randomGenerator_(std::random_device{}()),
      fluctuationDist_(-1.0, 1.0)
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

    // 2. 根据当前状态调用相应的更新函数（同步两台发动机）
    SystemState currentState = systemData_.leftEngine.state;

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

    // 5. 同步两台发动机
    synchronizeEngines();
}

// ==================== 控制接口函数 ====================

void EngineSimulator::startEngine()
{
    // 1. 检查当前状态是否为OFF
    if (systemData_.leftEngine.state != SystemState::OFF)
    {
        return; // 已经启动，不重复启动
    }

    // 2. 切换到STARTING_P1状态
    systemData_.leftEngine.state = SystemState::STARTING_P1;
    systemData_.rightEngine.state = SystemState::STARTING_P1;

    // 3. 重置启动计时器
    startingTimer_ = 0.0;

    // 4. 初始化相关参数
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

    // 2. 修改燃油流速V（±1单位/秒）
    systemData_.fuel.flowRate += direction * 1.0;
    systemData_.fuel.flowRate = clamp(systemData_.fuel.flowRate,
                                      Constants::FUEL_FLOW_MIN,
                                      Constants::FUEL_FLOW_MAX);

    // 3. 随机调整N1和EGT（3%-5%范围）
    double randomFactor = 0.03 + (fluctuationDist_(randomGenerator_) + 1.0) * 0.01; // 3%-5%
    double n1Adjustment = direction * randomFactor * systemData_.leftEngine.n1Percentage;
    double egtAdjustment = direction * randomFactor * systemData_.leftEngine.egtTemperature;

    systemData_.leftEngine.n1Percentage += n1Adjustment;
    systemData_.leftEngine.egtTemperature += egtAdjustment;
    systemData_.rightEngine.n1Percentage += n1Adjustment;
    systemData_.rightEngine.egtTemperature += egtAdjustment;

    // 限制在有效范围内
    systemData_.leftEngine.n1Percentage = clamp(systemData_.leftEngine.n1Percentage, 0, 125);
    systemData_.rightEngine.n1Percentage = clamp(systemData_.rightEngine.n1Percentage, 0, 125);
    systemData_.leftEngine.egtTemperature = clamp(systemData_.leftEngine.egtTemperature,
                                                  Constants::T0_AMBIENT,
                                                  Constants::EGT_MAX);
    systemData_.rightEngine.egtTemperature = clamp(systemData_.rightEngine.egtTemperature,
                                                   Constants::T0_AMBIENT,
                                                   Constants::EGT_MAX);
}

void EngineSimulator::injectFault(EngineID engineID, FaultType faultType)
{
    // TODO: 实现故障注入（进阶功能）
    // 1. 根据engineID选择目标发动机
    // 2. 根据faultType设置对应传感器的valid标志为false
    // 3. 或触发特定的异常状态
}

void EngineSimulator::clearFault(EngineID engineID)
{
    // TODO: 实现故障清除
    // 1. 根据engineID选择目标发动机
    // 2. 恢复所有传感器的valid标志为true
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

// ==================== 私有辅助函数 ====================

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

    // 添加小幅随机波动
    targetN1 = addFluctuation(targetN1, 0.01);
    targetEGT = addFluctuation(targetEGT, 0.01);
    targetFlow = addFluctuation(targetFlow, 0.01);

    // 更新数值
    systemData_.leftEngine.n1Percentage = targetN1;
    systemData_.leftEngine.egtTemperature = targetEGT;
    systemData_.rightEngine.n1Percentage = targetN1;
    systemData_.rightEngine.egtTemperature = targetEGT;
    systemData_.fuel.flowRate = targetFlow;

    // 达到95%额定转速后切换到RUNNING
    if (systemData_.leftEngine.n1Percentage >= Constants::N1_STABLE_THRESHOLD)
    {
        systemData_.leftEngine.state = SystemState::RUNNING;
        systemData_.rightEngine.state = SystemState::RUNNING;
    }
}

void EngineSimulator::updateRunningPhase(double dt)
{
    // 保持数值基本稳定，添加±3%的随机波动
    // 只对燃油流速添加波动，N1和EGT保持稳定（避免状态灯闪烁）

    // 燃油流速小幅波动
    double baseFlow = systemData_.fuel.flowRate;
    systemData_.fuel.flowRate = addFluctuation(baseFlow, Constants::FLUCTUATION_RANGE);
    systemData_.fuel.flowRate = clamp(systemData_.fuel.flowRate,
                                      Constants::FUEL_FLOW_MIN,
                                      Constants::FUEL_FLOW_MAX);

    // N1和EGT保持稳定（可选：添加微小波动）
    // systemData_.leftEngine.n1Percentage = addFluctuation(systemData_.leftEngine.n1Percentage, 0.005);
    // systemData_.leftEngine.egtTemperature = addFluctuation(systemData_.leftEngine.egtTemperature, 0.005);
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
