#ifndef ENGINE_SIMULATOR_H
#define ENGINE_SIMULATOR_H

#include "GlobalConstants.h"
#include <random>

/**
 * @class EngineSimulator
 * @brief 发动机仿真引擎类
 *
 * 负责物理逻辑模拟，根据当前状态计算下一时刻的各项数值
 * 包括转速、温度、燃油等参数的动态变化
 */
class EngineSimulator
{
public:
    // ==================== 构造与析构 ====================

    /**
     * @brief 构造函数
     */
    EngineSimulator();

    /**
     * @brief 析构函数
     */
    ~EngineSimulator();

    // ==================== 核心更新函数 ====================

    /**
     * @brief 核心逻辑函数，每5ms调用一次
     * @param dt 时间步长（秒）
     *
     * 根据当前SystemState执行不同的物理公式
     * - OFF: 无操作
     * - STARTING_P1: 线性增长（前2秒）
     * - STARTING_P2: 对数增长（至95%额定转速）
     * - RUNNING: 平稳波动（±3%）
     * - STOPPING: 对数下降（10秒内停止）
     */
    void update(double dt);

    // ==================== 控制接口函数 ====================

    /**
     * @brief 启动发动机
     *
     * 将状态从OFF转为STARTING_P1
     * 初始化启动阶段的相关参数
     */
    void startEngine();

    /**
     * @brief 停止发动机（最高优先级）
     *
     * 强制进入STOPPING状态
     * 立即将燃油流速归零，转速和温度对数下降
     */
    void stopEngine();

    /**
     * @brief 调整推力
     * @param direction 方向（+1增加推力，-1减小推力）
     *
     * 每次调整时：
     * - 燃油流速V增加/减少1单位/秒
     * - N1, EGT在3%-5%范围内随机增加/减少
     */
    void adjustThrust(int direction);

    /**
     * @brief 注入故障（进阶功能）
     * @param engineID 目标发动机ID
     * @param faultType 故障类型
     *
     * 手动标记某个传感器为"无效"或触发异常状态
     */
    void injectFault(EngineID engineID, FaultType faultType);

    /**
     * @brief 清除故障
     * @param engineID 目标发动机ID
     *
     * 恢复所有传感器为正常状态
     */
    void clearFault(EngineID engineID);

    // ==================== 数据访问接口 ====================

    /**
     * @brief 获取最新的系统数据
     * @return 当前时刻的SystemData结构体
     *
     * 供UI和AlertManager模块获取当前物理数据
     */
    SystemData getLatestData() const;

    /**
     * @brief 获取特定发动机的数据
     * @param engineID 发动机ID
     * @return 指定发动机的EngineData结构体
     */
    EngineData getEngineData(EngineID engineID) const;

    /**
     * @brief 获取燃油系统数据
     * @return 当前的FuelData结构体
     */
    FuelData getFuelData() const;

    /**
     * @brief 获取系统运行时间
     * @return 从启动开始的累计时间（秒）
     */
    double getElapsedTime() const;

    // ==================== 状态查询接口 ====================

    /**
     * @brief 检查发动机是否在运行
     * @return true表示发动机正在运行（RUNNING状态）
     */
    bool isRunning() const;

    /**
     * @brief 检查发动机是否在启动中
     * @return true表示发动机正在启动（STARTING_P1或STARTING_P2）
     */
    bool isStarting() const;

    /**
     * @brief 检查发动机是否在停车中
     * @return true表示发动机正在停车（STOPPING状态）
     */
    bool isStopping() const;

private:
    // ==================== 私有成员变量 ====================

    SystemData systemData_; // 系统整体数据
    double startingTimer_;  // 启动阶段计时器
    double stoppingTimer_;  // 停车阶段计时器
    double thrustLevel_;    // 推力级别（影响稳态数值）

    // 随机数生成器（用于模拟真实波动）
    std::mt19937 randomGenerator_;
    std::uniform_real_distribution<double> fluctuationDist_;
    // 当前注入的故障类型
    FaultType currentFaultType_;
    EngineID currentFaultEngineID_;
    // ==================== 私有辅助函数 ====================

    /**
     * @brief 更新启动阶段1（线性增长）
     * @param dt 时间步长
     */
    void updateStartingPhase1(double dt);

    /**
     * @brief 更新启动阶段2（对数增长）
     * @param dt 时间步长
     */
    void updateStartingPhase2(double dt);

    /**
     * @brief 更新稳定运行阶段
     * @param dt 时间步长
     */
    void updateRunningPhase(double dt);

    /**
     * @brief 更新停车阶段
     * @param dt 时间步长
     */
    void updateStoppingPhase(double dt);

    /**
     * @brief 更新燃油消耗
     * @param dt 时间步长
     *
     * 根据当前燃油流速计算燃油余量的减少
     */
    void updateFuelConsumption(double dt);

    /**
     * @brief 计算N1转速（对数增长公式）
     * @param t 时间参数
     * @return 计算得到的N1百分比
     *
     * 公式：N = 23000 * lg(t-1) + 20000
     */
    double calculateN1Logarithmic(double t);

    /**
     * @brief 计算EGT温度（对数增长公式）
     * @param t 时间参数
     * @return 计算得到的温度值（℃）
     *
     * 公式：T = 900 * lg(t-1) + T0
     */
    double calculateEGTLogarithmic(double t);

    /**
     * @brief 计算燃油流速（对数增长公式）
     * @param t 时间参数
     * @return 计算得到的流速值（单位/秒）
     *
     * 公式：V = 42 * lg(t-1) + 10
     */
    double calculateFuelFlowLogarithmic(double t);

    /**
     * @brief 添加随机波动
     * @param baseValue 基础值
     * @param range 波动范围（百分比）
     * @return 添加波动后的值
     */
    double addFluctuation(double baseValue, double range);

    /**
     * @brief 限制数值在有效范围内
     * @param value 待限制的值
     * @param minVal 最小值
     * @param maxVal 最大值
     * @return 限制后的值
     */
    double clamp(double value, double minVal, double maxVal);

    /**
     * @brief 更新传感器读数
     * @param engineData 发动机数据引用
     *
     * 将计算出的实际值分配给双冗余传感器
     * 考虑传感器故障的情况
     */
    void updateSensorReadings(EngineData &engineData);

    /**
     * @brief 同步两台发动机的状态
     *
     * 确保左右发动机同步启动和停止
     */
    void synchronizeEngines();
};

#endif // ENGINE_SIMULATOR_H
