#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#include <string>

// ==================== 枚举定义 ====================

// 系统状态枚举
enum class SystemState
{
    OFF,         // 关闭状态
    STARTING_P1, // 启动阶段1：线性增长（前2秒）
    STARTING_P2, // 启动阶段2：对数增长（至95%额定转速）
    RUNNING,     // 稳定运行状态
    STOPPING     // 停车阶段
};

// 告警级别枚举
enum class AlertLevel
{
    NORMAL,   // 正常 - 白色
    ADVISORY, // 建议 - 白色告警
    CAUTION,  // 警戒 - 琥珀色
    WARNING,  // 警告 - 红色
    INVALID   // 无效 - 显示"--"
};

// 发动机ID枚举
enum class EngineID
{
    LEFT = 0, // 左发动机
    RIGHT = 1 // 右发动机
};

// 按钮ID枚举
enum class ButtonID
{
    START,           // 启动按钮
    STOP,            // 停车按钮
    INCREASE_THRUST, // 增加推力按钮
    DECREASE_THRUST  // 减小推力按钮
};

// 故障类型枚举（进阶项用）
enum class FaultType
{
    NONE = 0,
    // 传感器故障
    SINGLE_N1_SENSOR_FAULT,   // 单个转速传感器故障
    SINGLE_ENGINE_N1_FAULT,   // 单发转速传感器故障（双传感器）
    SINGLE_EGT_SENSOR_FAULT,  // 单个EGT传感器故障
    SINGLE_ENGINE_EGT_FAULT,  // 单发EGT传感器故障（双传感器）
    DUAL_ENGINE_SENSOR_FAULT, // 双发传感器故障
    // 燃油故障
    FUEL_LOW,          // 燃油余量低
    FUEL_SENSOR_FAULT, // 燃油传感器故障
    FUEL_FLOW_EXCEED,  // 燃油流速超限
    // 转速异常
    OVERSPEED_1, // 超转1（N1 > 105%）
    OVERSPEED_2, // 超转2（N1 > 120%）
    // 温度异常
    OVERTEMP_1_STARTING, // 启动超温1（T > 850℃）
    OVERTEMP_2_STARTING, // 启动超温2（T > 1000℃）
    OVERTEMP_3_RUNNING,  // 稳态超温3（T > 950℃）
    OVERTEMP_4_RUNNING   // 稳态超温4（T > 1100℃）
};

// ==================== 结构体定义 ====================

// 传感器数据结构
struct SensorData
{
    double value1; // 传感器1的值
    double value2; // 传感器2的值
    bool valid1;   // 传感器1是否有效
    bool valid2;   // 传感器2是否有效

    SensorData() : value1(0.0), value2(0.0), valid1(true), valid2(true) {}
};

// 单个发动机数据结构
struct EngineData
{
    EngineID engineID;     // 发动机ID
    SensorData n1Sensors;  // 转速传感器（双冗余）
    SensorData egtSensors; // EGT温度传感器（双冗余）
    double n1Percentage;   // N1转速百分比（0-125%）
    double egtTemperature; // EGT温度（℃）
    SystemState state;     // 发动机状态

    EngineData() : engineID(EngineID::LEFT),
                   n1Percentage(0.0),
                   egtTemperature(20.0),
                   state(SystemState::OFF) {}
};

// 燃油系统数据结构（全局共享）
struct FuelData
{
    double capacity;  // 燃油余量C（0-20000单位）
    double flowRate;  // 燃油流速V（单位/秒）
    bool sensorValid; // 燃油传感器是否有效

    FuelData() : capacity(20000.0), flowRate(0.0), sensorValid(true) {}
};

// 系统整体数据结构
struct SystemData
{
    EngineData leftEngine;  // 左发动机数据
    EngineData rightEngine; // 右发动机数据
    FuelData fuel;          // 燃油系统数据
    double elapsedTime;     // 系统运行时间（秒）

    SystemData() : elapsedTime(0.0)
    {
        leftEngine.engineID = EngineID::LEFT;
        rightEngine.engineID = EngineID::RIGHT;
    }
};

// ==================== 全局常量定义 ====================

namespace Constants
{
    // 时间相关
    const double TIME_STEP = 0.005;        // 时间步长：5ms = 0.005s
    const double REFRESH_INTERVAL = 0.005; // 界面刷新间隔：5ms

    // 发动机参数
    const double RATED_RPM = 40000.0;        // 额定转速：40000转/分
    const double N1_MIN = 0.0;               // N1最小值：0%
    const double N1_MAX = 125.0;             // N1最大值：125%
    const double N1_STABLE_THRESHOLD = 95.0; // 稳态阈值：95%

    // 温度参数
    const double T0_AMBIENT = 20.0; // 环境温度：20℃
    const double EGT_MIN = -5.0;    // EGT最小值：-5℃
    const double EGT_MAX = 1200.0;  // EGT最大值：1200℃

    // 燃油参数
    const double FUEL_MAX = 20000.0;   // 最大燃油容量：20000单位
    const double FUEL_MIN = 0.0;       // 最小燃油容量：0单位
    const double FUEL_FLOW_MAX = 50.0; // 最大燃油流速：50单位/秒
    const double FUEL_FLOW_MIN = 0.0;  // 最小燃油流速：0单位/秒

    // 启动阶段参数
    const double PHASE1_DURATION = 2.0;    // 阶段1持续时间：2秒
    const double PHASE1_N1_RATE = 10000.0; // 阶段1转速增长率：10^4转/秒
    const double PHASE1_FUEL_RATE = 5.0;   // 阶段1燃油流速增长率：5单位/秒²

    // 停车阶段参数
    const double STOPPING_DURATION = 10.0; // 停车持续时间：10秒

    // 告警阈值
    const double FUEL_LOW_THRESHOLD = 1000.0; // 燃油低告警阈值：1000单位
    const double N1_CAUTION = 105.0;          // 超转1告警阈值：105%
    const double N1_WARNING = 120.0;          // 超转2告警阈值：120%
    const double EGT_CAUTION_START = 850.0;   // 启动超温1告警：850℃
    const double EGT_WARNING_START = 1000.0;  // 启动超温2告警：1000℃
    const double EGT_CAUTION_RUN = 950.0;     // 稳态超温3告警：950℃
    const double EGT_WARNING_RUN = 1100.0;    // 稳态超温4告警：1100℃

    // 波动范围
    const double FLUCTUATION_RANGE = 0.03; // 稳态波动范围：±3%

    // 告警显示时间
    const double ALERT_DISPLAY_DURATION = 5.0; // 告警显示持续时间：5秒

    // UI相关
    const double GAUGE_ANGLE_MIN = 0.0;   // 表盘最小角度：0°
    const double GAUGE_ANGLE_MAX = 210.0; // 表盘最大角度：210°
}

#endif // GLOBAL_CONSTANTS_H
