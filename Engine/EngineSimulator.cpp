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
    // TODO: 实现核心更新逻辑
    // 1. 更新运行时间
    // 2. 根据当前状态调用相应的更新函数
    // 3. 更新燃油消耗
    // 4. 更新传感器读数
}

// ==================== 控制接口函数 ====================

void EngineSimulator::startEngine()
{
    // TODO: 实现启动逻辑
    // 1. 检查当前状态是否为OFF
    // 2. 切换到STARTING_P1状态
    // 3. 重置启动计时器
    // 4. 初始化相关参数
}

void EngineSimulator::stopEngine()
{
    // TODO: 实现停车逻辑（最高优先级）
    // 1. 立即切换到STOPPING状态
    // 2. 燃油流速归零
    // 3. 重置停车计时器
}

void EngineSimulator::adjustThrust(int direction)
{
    // TODO: 实现推力调整
    // 1. 根据direction调整thrustLevel_
    // 2. 修改燃油流速V（±1单位/秒）
    // 3. 随机调整N1和EGT（3%-5%范围）
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
    // TODO: 实现启动阶段1的更新逻辑
    // 线性增长：N1以每秒10^4转速度增加
    // 燃油流速以每秒5单位速度增加
    // 持续2秒后切换到STARTING_P2
}

void EngineSimulator::updateStartingPhase2(double dt)
{
    // TODO: 实现启动阶段2的更新逻辑
    // 对数增长：使用对数函数计算N1、EGT、燃油流速
    // 达到95%额定转速后切换到RUNNING
}

void EngineSimulator::updateRunningPhase(double dt)
{
    // TODO: 实现稳定运行阶段的更新逻辑
    // 保持数值基本稳定，添加±3%的随机波动
}

void EngineSimulator::updateStoppingPhase(double dt)
{
    // TODO: 实现停车阶段的更新逻辑
    // 燃油流速已归零
    // N1和EGT以对数方式下降，10秒内降至0
}

void EngineSimulator::updateFuelConsumption(double dt)
{
    // TODO: 实现燃油消耗更新
    // 计算公式：ΔC = -V * dt
    // 确保燃油余量不低于0
}

double EngineSimulator::calculateN1Logarithmic(double t)
{
    // TODO: 实现N1对数计算公式
    // N = 23000 * lg(t-1) + 20000
    // 注意：lg表示以10为底的对数
    return 0.0; // 占位返回
}

double EngineSimulator::calculateEGTLogarithmic(double t)
{
    // TODO: 实现EGT对数计算公式
    // T = 900 * lg(t-1) + T0
    return Constants::T0_AMBIENT; // 占位返回
}

double EngineSimulator::calculateFuelFlowLogarithmic(double t)
{
    // TODO: 实现燃油流速对数计算公式
    // V = 42 * lg(t-1) + 10
    return 0.0; // 占位返回
}

double EngineSimulator::addFluctuation(double baseValue, double range)
{
    // TODO: 实现随机波动
    // 生成[-range, +range]范围内的随机百分比
    // 返回baseValue * (1 + 随机百分比)
    return baseValue; // 占位返回
}

double EngineSimulator::clamp(double value, double minVal, double maxVal)
{
    return std::max(minVal, std::min(value, maxVal));
}

void EngineSimulator::updateSensorReadings(EngineData &engineData)
{
    // TODO: 实现传感器读数更新
    // 1. 将计算的实际值分配给sensor1和sensor2
    // 2. 如果传感器失效，设置对应的值为特殊标记
    // 3. 可以为两个传感器添加微小的差异以增加真实性
}

void EngineSimulator::synchronizeEngines()
{
    // TODO: 实现发动机同步
    // 确保左右发动机状态一致（可选择是否完全同步）
}
