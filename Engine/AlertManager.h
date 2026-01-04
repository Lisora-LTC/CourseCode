#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include "GlobalConstants.h"
#include <vector>
#include <string>
#include <map>

/**
 * @struct AlertInfo
 * @brief 告警信息结构体
 */
struct AlertInfo
{
    FaultType faultType; // 故障类型
    AlertLevel level;    // 告警级别
    std::string message; // 告警消息（英文）
    double timestamp;    // 告警触发时间
    bool isActive;       // 是否活跃
    bool updated;        // 本帧是否更新
    double displayTimer; // 显示计时器（5秒）

    AlertInfo() : faultType(FaultType::NONE),
                  level(AlertLevel::NORMAL),
                  message(""),
                  timestamp(0.0),
                  isActive(false),
                  updated(false),
                  displayTimer(0.0) {}
};

/**
 * @class AlertManager
 * @brief 告警管理类
 *
 * 负责检测系统异常，管理14种故障类型的告警
 * 处理告警的生命周期和显示逻辑
 */
class AlertManager
{
public:
    // ==================== 构造与析构 ====================

    /**
     * @brief 构造函数
     */
    AlertManager();

    /**
     * @brief 析构函数
     */
    ~AlertManager();

    // ==================== 核心检测函数 ====================

    /**
     * @brief 检查系统状态并返回最高级别告警
     * @param data 系统数据
     * @return 当前最高级别的AlertLevel
     *
     * 根据14种异常规则进行判断：
     * - 传感器故障（6种）
     * - 燃油异常（3种）
     * - 转速异常（2种）
     * - 温度异常（4种）
     */
    AlertLevel checkCondition(const SystemData &data);

    /**
     * @brief 更新告警计时器
     * @param dt 时间步长（秒）
     *
     * 管理5秒显示逻辑
     * 确保同一告警在5秒内只记录一次Log
     */
    void updateTimers(double dt);

    // ==================== 数据访问接口 ====================

    /**
     * @brief 获取当前需要显示的告警消息列表
     * @return 英文告警消息的vector
     *
     * 只返回displayTimer > 0的活跃告警
     */
    std::vector<std::string> getActiveMessages() const;

    /**
     * @brief 获取所有告警信息
     * @return 完整的告警信息列表
     */
    std::vector<AlertInfo> getAllAlerts() const;

    /**
     * @brief 获取当前最高告警级别
     * @return 最高的AlertLevel
     */
    AlertLevel getHighestAlertLevel() const;

    /**
     * @brief 获取新触发的告警（用于日志记录）
     * @return 新触发的告警列表（上次调用后新增的）
     */
    std::vector<AlertInfo> getNewAlerts();

    /**
     * @brief 清除所有告警
     */
    void clearAllAlerts();

    /**
     * @brief 检查指定类型的告警是否处于活跃状态
     * @param faultType 故障类型
     * @param message 告警消息（可选，用于区分同类型的不同告警）
     * @return 如果告警活跃返回true
     */
    bool isAlertActive(FaultType faultType, const std::string &message = "") const;

private:
    // ==================== 私有成员变量 ====================

    std::vector<AlertInfo> activeAlerts_; // 活跃告警列表
    std::vector<AlertInfo> newAlerts_;    // 新触发的告警列表
    AlertLevel highestLevel_;             // 当前最高告警级别

    // 用于防止5秒内重复记录日志
    std::map<FaultType, double> lastLogTime_; // 每种故障类型的最后记录时间

    // ==================== 私有检测函数 ====================

    /**
     * @brief 检查传感器故障（6种）
     * @param data 系统数据
     *
     * 1. 单个转速传感器故障（单发单传感器）- 白色
     * 2. 单发转速传感器故障（单发双传感器）- 琥珀色
     * 3. 单个EGT传感器故障（单发单传感器）- 白色
     * 4. 单发EGT传感器故障（单发双传感器）- 琥珀色
     * 5. 双发转速传感器故障 - 红色，停车
     * 6. 双发EGT传感器故障 - 红色，停车
     */
    void checkSensorFaults(const SystemData &data);

    /**
     * @brief 检查燃油异常（3种）
     * @param data 系统数据
     *
     * 1. 燃油余量低于1000 - 琥珀色
     * 2. 燃油传感器故障 - 红色
     * 3. 燃油流速超过50 - 琥珀色
     */
    void checkFuelAbnormal(const SystemData &data);

    /**
     * @brief 检查转速异常（2种）
     * @param data 系统数据
     *
     * 1. 超转1：N1 > 105% - 琥珀色
     * 2. 超转2：N1 > 120% - 红色，停车
     */
    void checkThrustAbnormal(const SystemData &data);

    /**
     * @brief 检查温度异常（4种）
     * @param data 系统数据
     *
     * 1. 超温1：启动时EGT > 850℃ - 琥珀色
     * 2. 超温2：启动时EGT > 1000℃ - 红色，停车
     * 3. 超温3：稳态时EGT > 950℃ - 琥珀色
     * 4. 超温4：稳态时EGT > 1100℃ - 红色，停车
     */
    void checkTempAbnormal(const SystemData &data);

    // ==================== 私有辅助函数 ====================

    /**
     * @brief 添加告警
     * @param faultType 故障类型
     * @param level 告警级别
     * @param message 告警消息
     * @param timestamp 时间戳
     */
    void addAlert(FaultType faultType, AlertLevel level,
                  const std::string &message, double timestamp);

    /**
     * @brief 检查告警是否已存在
     * @param faultType 故障类型
     * @return true表示已存在
     */
    bool alertExists(FaultType faultType) const;

    /**
     * @brief 移除指定类型的告警
     * @param faultType 故障类型
     */
    void removeAlert(FaultType faultType);

    /**
     * @brief 检查是否需要记录日志
     * @param faultType 故障类型
     * @param currentTime 当前时间
     * @return true表示需要记录
     *
     * 5秒内同一种告警只记录一次
     */
    bool shouldLogAlert(FaultType faultType, double currentTime);

    /**
     * @brief 更新最高告警级别
     */
    void updateHighestLevel();

    /**
     * @brief 判断发动机是否在启动阶段
     * @param state 系统状态
     * @return true表示在启动阶段
     */
    bool isStartingPhase(SystemState state) const;

    /**
     * @brief 判断发动机是否在稳态运行
     * @param state 系统状态
     * @return true表示在稳态运行
     */
    bool isRunningPhase(SystemState state) const;

    /**
     * @brief 检查单个发动机的传感器状态
     * @param engine 发动机数据
     * @param engineID 发动机ID
     */
    void checkEngineSensors(const EngineData &engine, EngineID engineID);

    /**
     * @brief 生成告警消息文本
     * @param faultType 故障类型
     * @param engineID 发动机ID（如果适用）
     * @return 英文告警消息
     */
    std::string generateAlertMessage(FaultType faultType,
                                     EngineID engineID = EngineID::LEFT) const;
};

#endif // ALERT_MANAGER_H
