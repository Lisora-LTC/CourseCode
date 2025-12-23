#include "Logger.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <filesystem>

// ==================== 构造与析构 ====================

Logger::Logger(const std::string &baseDir)
    : baseDir_(baseDir),
      filesOpen_(false)
{
    // 初始化
}

Logger::~Logger()
{
    closeFiles();
}

// ==================== 初始化与清理 ====================

bool Logger::initFiles()
{
    // TODO: 实现文件初始化
    // 1. 确保基础目录存在
    // 2. 生成时间戳文件名
    // 3. 打开CSV文件和Log文件
    // 4. 写入文件表头
    // 5. 设置filesOpen_标志

    return false; // 占位返回
}

void Logger::closeFiles()
{
    // TODO: 实现文件关闭
    // 1. 刷新缓冲区
    // 2. 关闭CSV文件
    // 3. 关闭Log文件
    // 4. 设置filesOpen_为false
}

// ==================== 数据记录接口 ====================

void Logger::recordData(double timestamp, const SystemData &data)
{
    // TODO: 实现数据记录
    // 1. 检查文件是否打开
    // 2. 格式化时间戳
    // 3. 写入CSV行：
    //    timestamp, 左发N1传感器1, 左发N1传感器2, 左发EGT传感器1, 左发EGT传感器2,
    //    右发N1传感器1, 右发N1传感器2, 右发EGT传感器1, 右发EGT传感器2,
    //    燃油余量, 燃油流速
    // 4. 使用formatSensorData处理传感器数据
}

void Logger::recordEvent(double timestamp, const std::string &message)
{
    // TODO: 实现事件记录
    // 1. 检查文件是否打开
    // 2. 格式化时间戳为[HH:MM:SS.mmm]
    // 3. 写入日志：[timestamp] message
    // 4. 换行
}

void Logger::recordAlert(double timestamp, const AlertInfo &alert)
{
    // TODO: 实现告警记录
    // 1. 将AlertInfo转换为可读文本
    // 2. 包括告警级别、消息内容
    // 3. 调用recordEvent
}

void Logger::recordEvents(double timestamp, const std::vector<std::string> &messages)
{
    // TODO: 实现批量事件记录
    // 遍历messages，逐个调用recordEvent
}

// ==================== 状态查询接口 ====================

bool Logger::isOpen() const
{
    return filesOpen_;
}

std::string Logger::getCSVFilePath() const
{
    return csvFilePath_;
}

std::string Logger::getLogFilePath() const
{
    return logFilePath_;
}

void Logger::flush()
{
    if (csvFile_.is_open())
    {
        csvFile_.flush();
    }
    if (logFile_.is_open())
    {
        logFile_.flush();
    }
}

// ==================== 私有辅助函数 ====================

std::string Logger::generateTimestampFilename(const std::string &extension) const
{
    // TODO: 实现时间戳文件名生成
    // 1. 获取当前系统时间
    // 2. 格式化为YYYYMMDD_HHMMSS
    // 3. 拼接：EICAS_ + 时间戳 + extension

    return "EICAS_temp" + extension; // 占位返回
}

void Logger::writeCSVHeader()
{
    // TODO: 实现CSV表头写入
    // 写入列名：
    // Time,L_N1_S1,L_N1_S2,L_N1_Valid1,L_N1_Valid2,L_EGT_S1,L_EGT_S2,L_EGT_Valid1,L_EGT_Valid2,
    // R_N1_S1,R_N1_S2,R_N1_Valid1,R_N1_Valid2,R_EGT_S1,R_EGT_S2,R_EGT_Valid1,R_EGT_Valid2,
    // Fuel_Capacity,Fuel_FlowRate,Fuel_Valid
}

void Logger::writeLogHeader()
{
    // TODO: 实现Log表头写入
    // 写入启动信息：
    // ===================================
    // Engine Monitoring System Log
    // Start Time: YYYY-MM-DD HH:MM:SS
    // ===================================
}

std::string Logger::formatTimestamp(double timestamp) const
{
    // 将秒数转换为HH:MM:SS.mmm格式
    int hours = static_cast<int>(timestamp) / 3600;
    int minutes = (static_cast<int>(timestamp) % 3600) / 60;
    int seconds = static_cast<int>(timestamp) % 60;
    int milliseconds = static_cast<int>((timestamp - static_cast<int>(timestamp)) * 1000);

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setw(2) << minutes << ":"
        << std::setw(2) << seconds << "."
        << std::setw(3) << milliseconds;

    return oss.str();
}

std::string Logger::getCurrentTimeString() const
{
    // TODO: 实现当前时间字符串生成
    // 使用<chrono>和<ctime>获取当前系统时间
    // 格式化为：YYYY-MM-DD HH:MM:SS

    return "2025-01-01 00:00:00"; // 占位返回
}

std::string Logger::alertLevelToString(AlertLevel level) const
{
    switch (level)
    {
    case AlertLevel::NORMAL:
        return "NORMAL";
    case AlertLevel::ADVISORY:
        return "ADVISORY";
    case AlertLevel::CAUTION:
        return "CAUTION";
    case AlertLevel::WARNING:
        return "WARNING";
    case AlertLevel::INVALID:
        return "INVALID";
    default:
        return "UNKNOWN";
    }
}

std::string Logger::formatSensorData(const SensorData &sensor) const
{
    // TODO: 实现传感器数据格式化
    // 返回：value1,value2,valid1,valid2
    // 如果传感器失效，值写入"N/A"

    std::ostringstream oss;
    // 示例：
    // oss << (sensor.valid1 ? std::to_string(sensor.value1) : "N/A") << ","
    //     << (sensor.valid2 ? std::to_string(sensor.value2) : "N/A") << ","
    //     << (sensor.valid1 ? "1" : "0") << ","
    //     << (sensor.valid2 ? "1" : "0");

    return oss.str();
}

bool Logger::ensureDirectoryExists(const std::string &path) const
{
    // TODO: 实现目录存在性检查和创建
    // 使用<filesystem>库
    // 1. 检查目录是否存在
    // 2. 如果不存在，尝试创建
    // 3. 返回是否成功

    return true; // 占位返回
}
