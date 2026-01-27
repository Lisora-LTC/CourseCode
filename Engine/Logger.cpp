#include "Logger.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <direct.h>
#include <io.h>

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
    // 1. 确保基础目录存在
    if (_access(baseDir_.c_str(), 0) != 0)
    {
        if (_mkdir(baseDir_.c_str()) != 0)
        {
            return false; // 创建目录失败
        }
    }

    // 2. 生成时间戳文件名
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    localtime_s(&tm_now, &time_t_now);

    std::ostringstream oss;
    oss << "EICAS_"
        << std::setfill('0')
        << std::setw(4) << (tm_now.tm_year + 1900)
        << std::setw(2) << (tm_now.tm_mon + 1)
        << std::setw(2) << tm_now.tm_mday << "_"
        << std::setw(2) << tm_now.tm_hour
        << std::setw(2) << tm_now.tm_min
        << std::setw(2) << tm_now.tm_sec;

    std::string baseName = oss.str();
    csvFilePath_ = baseDir_ + "/" + baseName + ".csv";
    logFilePath_ = baseDir_ + "/" + baseName + ".log";

    // 3. 打开CSV文件
    csvFile_.open(csvFilePath_, std::ios::out);
    if (!csvFile_.is_open())
    {
        return false;
    }

    // 4. 写入CSV表头
    csvFile_ << "Time,L_N1_S1,L_N1_S2,L_N1_Valid1,L_N1_Valid2,"
             << "L_EGT_S1,L_EGT_S2,L_EGT_Valid1,L_EGT_Valid2,"
             << "R_N1_S1,R_N1_S2,R_N1_Valid1,R_N1_Valid2,"
             << "R_EGT_S1,R_EGT_S2,R_EGT_Valid1,R_EGT_Valid2,"
             << "Fuel_Capacity,Fuel_FlowRate\n";

    // 5. 打开Log文件
    logFile_.open(logFilePath_, std::ios::out);
    if (!logFile_.is_open())
    {
        csvFile_.close();
        return false;
    }

    writeLogHeader();

    // 6. 设置标志
    filesOpen_ = true;
    return true;
}

void Logger::closeFiles()
{
    if (filesOpen_)
    {
        // 1. 刷新缓冲区
        if (csvFile_.is_open())
        {
            csvFile_.flush();
            csvFile_.close();
        }
        if (logFile_.is_open())
        {
            logFile_.flush();
            logFile_.close();
        }
        // 2. 设置标志
        filesOpen_ = false;
    }
}

// ==================== 数据记录接口 ====================

void Logger::recordData(double timestamp, const SystemData &data)
{
    if (!filesOpen_ || !csvFile_.is_open())
    {
        return;
    }

    // Time
    csvFile_ << std::fixed << std::setprecision(3) << timestamp << ",";

    // Left Engine N1
    csvFile_ << formatSensorData(data.leftEngine.n1Sensors) << ",";
    // Left Engine EGT
    csvFile_ << formatSensorData(data.leftEngine.egtSensors) << ",";

    // Right Engine N1
    csvFile_ << formatSensorData(data.rightEngine.n1Sensors) << ",";
    // Right Engine EGT
    csvFile_ << formatSensorData(data.rightEngine.egtSensors) << ",";

    // Fuel
    csvFile_ << std::fixed << std::setprecision(2) << data.fuel.capacity << ","
             << std::fixed << std::setprecision(2) << data.fuel.flowRate << "\n";
}

void Logger::recordEvent(double timestamp, const std::string &message)
{
    if (!filesOpen_ || !logFile_.is_open())
    {
        return;
    }

    logFile_ << "[" << formatTimestamp(timestamp) << "] " << message << "\n";
    logFile_.flush(); // Ensure event is written immediately
}

void Logger::recordAlert(double timestamp, const AlertInfo &alert)
{
    std::ostringstream oss;
    oss << alertLevelToString(alert.level) << ": " << alert.message;
    recordEvent(timestamp, oss.str());
}

void Logger::recordEvents(double timestamp, const std::vector<std::string> &messages)
{
    for (const auto &msg : messages)
    {
        recordEvent(timestamp, msg);
    }
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
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    localtime_s(&tm_now, &time_t_now);

    std::ostringstream oss;
    oss << "EICAS_"
        << std::setfill('0')
        << std::setw(4) << (tm_now.tm_year + 1900)
        << std::setw(2) << (tm_now.tm_mon + 1)
        << std::setw(2) << tm_now.tm_mday << "_"
        << std::setw(2) << tm_now.tm_hour
        << std::setw(2) << tm_now.tm_min
        << std::setw(2) << tm_now.tm_sec
        << extension;

    return oss.str();
}

void Logger::writeCSVHeader()
{
    if (csvFile_.is_open())
    {
        csvFile_ << "Time,L_N1_S1,L_N1_S2,L_N1_Valid1,L_N1_Valid2,"
                 << "L_EGT_S1,L_EGT_S2,L_EGT_Valid1,L_EGT_Valid2,"
                 << "R_N1_S1,R_N1_S2,R_N1_Valid1,R_N1_Valid2,"
                 << "R_EGT_S1,R_EGT_S2,R_EGT_Valid1,R_EGT_Valid2,"
                 << "Fuel_Capacity,Fuel_FlowRate\n";
    }
}

void Logger::writeLogHeader()
{
    if (logFile_.is_open())
    {
        logFile_ << "===================================\n"
                 << "Engine Monitoring System Log\n"
                 << "Start Time: " << getCurrentTimeString() << "\n"
                 << "===================================\n";
    }
}

std::string Logger::formatTimestamp(double timestamp) const
{
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
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    localtime_s(&tm_now, &time_t_now);

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << (tm_now.tm_year + 1900) << "-"
        << std::setw(2) << (tm_now.tm_mon + 1) << "-"
        << std::setw(2) << tm_now.tm_mday << " "
        << std::setw(2) << tm_now.tm_hour << ":"
        << std::setw(2) << tm_now.tm_min << ":"
        << std::setw(2) << tm_now.tm_sec;

    return oss.str();
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
    case AlertLevel::DANGER:
        return "DANGER";
    case AlertLevel::INVALID:
        return "INVALID";
    default:
        return "UNKNOWN";
    }
}

std::string Logger::formatSensorData(const SensorData &sensor) const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    if (sensor.valid1)
        oss << sensor.value1;
    else
        oss << "N/A";
    oss << ",";
    if (sensor.valid2)
        oss << sensor.value2;
    else
        oss << "N/A";
    oss << ",";
    oss << (sensor.valid1 ? "1" : "0") << ",";
    oss << (sensor.valid2 ? "1" : "0");

    return oss.str();
}

bool Logger::ensureDirectoryExists(const std::string &path) const
{
    if (_access(path.c_str(), 0) == 0)
    {
        return true;
    }
    return _mkdir(path.c_str()) == 0;
}
