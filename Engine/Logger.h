#ifndef LOGGER_H
#define LOGGER_H

#include "GlobalConstants.h"
#include "AlertManager.h"
#include <string>
#include <fstream>

/**
 * @class Logger
 * @brief 日志与数据持久化类
 *
 * 负责CSV数据记录和文本Log记录
 * 每次程序运行时创建新的日志文件
 */
class Logger
{
public:
    // ==================== 构造与析构 ====================

    /**
     * @brief 构造函数
     * @param baseDir 日志文件基础目录（默认为当前目录）
     */
    Logger(const std::string &baseDir = ".");

    /**
     * @brief 析构函数
     *
     * 自动关闭所有打开的文件
     */
    ~Logger();

    // ==================== 初始化与清理 ====================

    /**
     * @brief 初始化文件
     * @return true表示初始化成功
     *
     * 程序启动时调用，创建以时间命名的.csv和.log文件
     * 文件命名格式：EICAS_YYYYMMDD_HHMMSS.csv / .log
     */
    bool initFiles();

    /**
     * @brief 关闭文件
     *
     * 程序退出时调用，确保文件缓冲区全部刷入磁盘
     */
    void closeFiles();

    // ==================== 数据记录接口 ====================

    /**
     * @brief 记录数据到CSV文件
     * @param timestamp 运行时间（秒）
     * @param data 系统数据
     *
     * 每5ms调用一次，将所有传感器原始数据写入CSV
     * CSV格式（示例）：
     * Time, L_N1_S1, L_N1_S2, L_EGT_S1, L_EGT_S2, R_N1_S1, R_N1_S2, R_EGT_S1, R_EGT_S2, Fuel_C, Fuel_V
     */
    void recordData(double timestamp, const SystemData &data);

    /**
     * @brief 记录事件到Log文件
     * @param timestamp 运行时间（秒）
     * @param message 事件消息
     *
     * 当AlertManager检测到新异常时调用
     * Log格式（示例）：
     * [00:01:23.456] WARNING: OVERSPEED - N1 > 120%
     */
    void recordEvent(double timestamp, const std::string &message);

    /**
     * @brief 记录告警信息
     * @param timestamp 运行时间（秒）
     * @param alert 告警信息结构体
     *
     * 将AlertInfo转换为可读文本并记录
     */
    void recordAlert(double timestamp, const AlertInfo &alert);

    /**
     * @brief 批量记录多个事件
     * @param timestamp 运行时间（秒）
     * @param messages 消息列表
     */
    void recordEvents(double timestamp, const std::vector<std::string> &messages);

    // ==================== 状态查询接口 ====================

    /**
     * @brief 检查文件是否已打开
     * @return true表示文件已成功打开
     */
    bool isOpen() const;

    /**
     * @brief 获取CSV文件路径
     * @return CSV文件的完整路径
     */
    std::string getCSVFilePath() const;

    /**
     * @brief 获取Log文件路径
     * @return Log文件的完整路径
     */
    std::string getLogFilePath() const;

    /**
     * @brief 刷新文件缓冲区
     *
     * 立即将缓冲区内容写入磁盘
     * 在关键操作后调用以防止数据丢失
     */
    void flush();

private:
    // ==================== 私有成员变量 ====================

    std::string baseDir_;     // 日志文件基础目录
    std::string csvFilePath_; // CSV文件完整路径
    std::string logFilePath_; // Log文件完整路径

    std::ofstream csvFile_; // CSV文件流
    std::ofstream logFile_; // Log文件流

    bool filesOpen_; // 文件是否已打开

    // ==================== 私有辅助函数 ====================

    /**
     * @brief 生成时间戳文件名
     * @param extension 文件扩展名（如".csv"或".log"）
     * @return 格式化的文件名
     *
     * 格式：EICAS_YYYYMMDD_HHMMSS.extension
     */
    std::string generateTimestampFilename(const std::string &extension) const;

    /**
     * @brief 写入CSV表头
     *
     * 在CSV文件开头写入列名
     * 例如：Time,L_N1_S1,L_N1_S2,...
     */
    void writeCSVHeader();

    /**
     * @brief 写入Log表头
     *
     * 在Log文件开头写入启动信息
     * 包括启动时间、系统信息等
     */
    void writeLogHeader();

    /**
     * @brief 格式化时间戳为字符串
     * @param timestamp 时间戳（秒）
     * @return 格式化的时间字符串
     *
     * 格式：HH:MM:SS.mmm（小时:分钟:秒.毫秒）
     */
    std::string formatTimestamp(double timestamp) const;

    /**
     * @brief 格式化当前系统时间
     * @return 格式化的日期时间字符串
     *
     * 格式：YYYY-MM-DD HH:MM:SS
     */
    std::string getCurrentTimeString() const;

    /**
     * @brief 将AlertLevel转换为字符串
     * @param level 告警级别
     * @return 字符串表示
     */
    std::string alertLevelToString(AlertLevel level) const;

    /**
     * @brief 将传感器数据格式化为CSV字段
     * @param sensor 传感器数据
     * @return CSV格式字符串（逗号分隔）
     *
     * 如果传感器失效，写入"N/A"
     */
    std::string formatSensorData(const SensorData &sensor) const;

    /**
     * @brief 确保目录存在
     * @param path 目录路径
     * @return true表示目录存在或创建成功
     *
     * 如果目录不存在，尝试创建
     */
    bool ensureDirectoryExists(const std::string &path) const;
};

#endif // LOGGER_H
