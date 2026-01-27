#include "GlobalConstants.h"
#include "EngineSimulator.h"
#include "AlertManager.h"
#include "EngineUI.h"
#include "Logger.h"
#include <iostream>
#include <chrono>
#include <thread>

/**
 * @file main.cpp
 * @brief 虚拟发动机性能监控系统主程序
 *
 * 程序架构：
 * 1. 初始化所有模块（Simulator, AlertManager, UI, Logger）
 * 2. 设置高精度计时器（5ms间隔）
 * 3. 主循环：
 *    - 更新仿真引擎（物理计算）
 *    - 检测告警条件
 *    - 记录数据到CSV和Log
 *    - 更新UI显示（30Hz）
 *    - 处理用户输入
 * 4. 清理资源并退出
 */

// ==================== 全局变量 ====================

EngineSimulator *g_simulator = nullptr; // 仿真引擎
AlertManager *g_alertManager = nullptr; // 告警管理器
EngineUI *g_ui = nullptr;               // 用户界面
Logger *g_logger = nullptr;             // 日志记录器

// 故障注入循环索引
int g_sensorFaultIndex = 0; // 传感器故障索引 (0-5)
int g_fuelFaultIndex = 0;   // 燃油故障索引 (0-2)
int g_n1FaultIndex = 0;     // 转速故障索引 (0-1)
int g_tempFaultIndex = 0;   // 温度故障索引 (0-3)

bool g_running = true;                        // 主循环运行标志
double g_lastUIUpdateTime = 0.0;              // 上次UI更新时间
const double UI_UPDATE_INTERVAL = 1.0 / 30.0; // UI更新间隔（30Hz）

// ==================== 函数声明 ====================

/**
 * @brief 初始化所有模块
 * @return true表示初始化成功
 */
bool initializeSystem();

/**
 * @brief 清理所有模块
 */
void shutdownSystem();

/**
 * @brief 按钮点击回调函数
 * @param buttonID 按钮ID
 *
 * 处理各种按钮的点击事件
 */
void onButtonClicked(ButtonID buttonID);

/**
 * @brief 主循环更新函数
 * @param deltaTime 时间步长（秒）
 *
 * 每5ms调用一次，执行所有更新逻辑
 */
void update(double deltaTime);

/**
 * @brief 更新UI显示
 *
 * 以较低频率（30Hz）更新界面，避免过度占用资源
 */
void updateUI();

/**
 * @brief 高精度延时函数
 * @param milliseconds 延时时间（毫秒）
 *
 * 使用高精度计时器实现精确延时
 */
void preciseSleep(double milliseconds);

/**
 * @brief 获取当前时间戳（秒）
 * @return 高精度时间戳
 */
double getCurrentTime();

// ==================== 主函数 ====================

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "  Engine Monitoring System (EICAS)" << std::endl;
    std::cout << "  Virtual Engine Performance Monitor" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // 初始化系统
    if (!initializeSystem())
    {
        std::cerr << "Failed to initialize system!" << std::endl;
        return -1;
    }

    std::cout << "System initialized successfully." << std::endl;
    std::cout << "CSV File: " << g_logger->getCSVFilePath() << std::endl;
    std::cout << "Log File: " << g_logger->getLogFilePath() << std::endl;
    std::cout << std::endl;
    std::cout << "Press START button to begin engine startup..." << std::endl;
    std::cout << "Press ESC or close window to exit." << std::endl;
    std::cout << std::endl;

    // 主循环
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();

    while (g_running)
    {
        // 计算时间差
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastUpdateTime;
        double deltaTime = elapsed.count();

        // 如果时间步长接近5ms，执行更新
        if (deltaTime >= Constants::TIME_STEP)
        {
            update(deltaTime);
            lastUpdateTime = currentTime;
        }

        // 处理UI事件
        if (g_ui && !g_ui->processEvents())
        {
            g_running = false;
        }

        // 短暂休眠，避免占用100% CPU
        preciseSleep(1.0); // 休眠1ms
    }

    // 清理资源
    std::cout << std::endl;
    std::cout << "Shutting down system..." << std::endl;
    shutdownSystem();

    std::cout << "System shutdown complete." << std::endl;
    std::cout << "Data saved to files." << std::endl;

    return 0;
}

// ==================== 函数实现 ====================

bool initializeSystem()
{
    // 1. 创建EngineSimulator实例
    g_simulator = new EngineSimulator();
    if (!g_simulator)
    {
        std::cerr << "Failed to create EngineSimulator!" << std::endl;
        return false;
    }

    // 2. 创建AlertManager实例
    g_alertManager = new AlertManager();
    if (!g_alertManager)
    {
        std::cerr << "Failed to create AlertManager!" << std::endl;
        return false;
    }

    // 3. 创建Logger实例并初始化文件
    g_logger = new Logger(".");
    if (!g_logger || !g_logger->initFiles())
    {
        std::cerr << "Failed to initialize Logger!" << std::endl;
        return false;
    }

    // 4. 创建EngineUI实例并初始化图形界面
    g_ui = new EngineUI(1600, 900);
    if (!g_ui || !g_ui->initialize())
    {
        std::cerr << "Failed to initialize EngineUI!" << std::endl;
        return false;
    }

    // 5. 设置UI的按钮回调函数
    g_ui->setButtonCallback(onButtonClicked);

    return true;
}

void shutdownSystem()
{
    // 关闭Logger文件
    if (g_logger)
    {
        g_logger->closeFiles();
        delete g_logger;
        g_logger = nullptr;
    }

    // 关闭UI
    if (g_ui)
    {
        g_ui->shutdown();
        delete g_ui;
        g_ui = nullptr;
    }

    // 删除AlertManager
    if (g_alertManager)
    {
        delete g_alertManager;
        g_alertManager = nullptr;
    }

    // 删除Simulator
    if (g_simulator)
    {
        delete g_simulator;
        g_simulator = nullptr;
    }
}

void onButtonClicked(ButtonID buttonID)
{
    switch (buttonID)
    {
    case ButtonID::START:
        std::cout << "\n========================================" << std::endl;
        std::cout << "[" << g_simulator->getElapsedTime() << "s] ENGINE START INITIATED" << std::endl;
        std::cout << "Starting engine sequence..." << std::endl;
        std::cout << "========================================\n"
                  << std::endl;
        g_simulator->startEngine();
        break;

    case ButtonID::STOP:
        std::cout << "\n========================================" << std::endl;
        std::cout << "[" << g_simulator->getElapsedTime() << "s] ENGINE STOP COMMANDED" << std::endl;
        std::cout << "Initiating shutdown sequence (Priority)" << std::endl;
        std::cout << "========================================\n"
                  << std::endl;
        g_simulator->stopEngine();
        break;

    case ButtonID::INCREASE_THRUST:
        std::cout << "[" << g_simulator->getElapsedTime() << "s] THRUST INCREASED" << std::endl;
        std::cout << "   Fuel flow +1 unit/s, N1/EGT +3~5%" << std::endl;
        g_simulator->adjustThrust(+1);
        break;

    case ButtonID::DECREASE_THRUST:
        std::cout << "[" << g_simulator->getElapsedTime() << "s] THRUST DECREASED" << std::endl;
        std::cout << "   Fuel flow -1 unit/s, N1/EGT -3~5%" << std::endl;
        g_simulator->adjustThrust(-1);
        break;

    // ==================== 传感器故障 ====================
    case ButtonID::FAULT_SENSOR_N1_SINGLE:
        g_simulator->injectFault(EngineID::LEFT, FaultType::SINGLE_N1_SENSOR_FAULT);
        g_ui->setCurrentFaultStatus("#1 Single N1 Sensor Fault - ADVISORY (White)");
        break;
    case ButtonID::FAULT_SENSOR_N1_ENGINE:
        g_simulator->injectFault(EngineID::LEFT, FaultType::SINGLE_ENGINE_N1_FAULT);
        g_ui->setCurrentFaultStatus("#2 Single Engine N1 Fault - CAUTION (Amber)");
        break;
    case ButtonID::FAULT_SENSOR_EGT_SINGLE:
        g_simulator->injectFault(EngineID::LEFT, FaultType::SINGLE_EGT_SENSOR_FAULT);
        g_ui->setCurrentFaultStatus("#3 Single EGT Sensor Fault - ADVISORY (White)");
        break;
    case ButtonID::FAULT_SENSOR_EGT_ENGINE:
        g_simulator->injectFault(EngineID::LEFT, FaultType::SINGLE_ENGINE_EGT_FAULT);
        g_ui->setCurrentFaultStatus("#4 Single Engine EGT Fault - CAUTION (Amber)");
        break;
    case ButtonID::FAULT_SENSOR_DUAL:
        g_simulator->injectFault(EngineID::LEFT, FaultType::DUAL_ENGINE_SENSOR_FAULT);
        g_ui->setCurrentFaultStatus("#5 Dual Engine Sensor Fault - WARNING (Red)");
        break;

    // ==================== 燃油故障 ====================
    case ButtonID::FAULT_FUEL_LOW:
        g_simulator->injectFault(EngineID::LEFT, FaultType::FUEL_LOW);
        g_ui->setCurrentFaultStatus("#7 Fuel Low - CAUTION (Amber)");
        break;
    case ButtonID::FAULT_FUEL_SENSOR:
        g_simulator->injectFault(EngineID::LEFT, FaultType::FUEL_SENSOR_FAULT);
        g_ui->setCurrentFaultStatus("#8 Fuel Sensor Fault - WARNING (Red)");
        break;
    case ButtonID::FAULT_FUEL_FLOW:
        g_simulator->injectFault(EngineID::LEFT, FaultType::FUEL_FLOW_EXCEED); // or FUEL_FLOW_HIGH
        g_ui->setCurrentFaultStatus("#9 Fuel Flow Exceed - CAUTION (Amber)");
        break;

    // ==================== 转速故障 ====================
    case ButtonID::FAULT_N1_OVER_1:
        g_simulator->injectFault(EngineID::LEFT, FaultType::OVERSPEED_1);
        g_ui->setCurrentFaultStatus("#10 Overspeed 1 - CAUTION (Amber)");
        break;
    case ButtonID::FAULT_N1_OVER_2:
        g_simulator->injectFault(EngineID::LEFT, FaultType::OVERSPEED_2);
        g_ui->setCurrentFaultStatus("#11 Overspeed 2 - WARNING (Red)");
        break;

    // ==================== 温度故障 ====================
    case ButtonID::FAULT_TEMP_START_1:
        g_simulator->injectFault(EngineID::LEFT, FaultType::OVERTEMP_1_STARTING);
        g_ui->setCurrentFaultStatus("#12 Overtemp 1 Starting - CAUTION (Amber)");
        break;
    case ButtonID::FAULT_TEMP_START_2:
        g_simulator->injectFault(EngineID::LEFT, FaultType::OVERTEMP_2_STARTING);
        g_ui->setCurrentFaultStatus("#13 Overtemp 2 Starting - WARNING (Red)");
        break;
    case ButtonID::FAULT_TEMP_RUN_3:
        g_simulator->injectFault(EngineID::LEFT, FaultType::OVERTEMP_3_RUNNING);
        g_ui->setCurrentFaultStatus("#14 Overtemp 3 Running - CAUTION (Amber)");
        break;
    case ButtonID::FAULT_TEMP_RUN_4:
        g_simulator->injectFault(EngineID::LEFT, FaultType::OVERTEMP_4_RUNNING);
        g_ui->setCurrentFaultStatus("#15 Overtemp 4 Running - WARNING (Red)");
        break;

    case ButtonID::CLEAR_FAULT:
        std::cout << "\n========================================" << std::endl;
        std::cout << "[" << g_simulator->getElapsedTime() << "s] ALL FAULTS CLEARED" << std::endl;
        std::cout << "System reset to normal operation" << std::endl;
        std::cout << "========================================\n"
                  << std::endl;

        g_simulator->clearFault(EngineID::LEFT);
        g_simulator->clearFault(EngineID::RIGHT);
        g_ui->setCurrentFaultStatus("No Fault Injected");

        // 重置循环索引 (虽然现在不用了，但保留也无妨)
        g_sensorFaultIndex = 0;
        g_fuelFaultIndex = 0;
        g_n1FaultIndex = 0;
        g_tempFaultIndex = 0;
        break;
    }
}

void update(double deltaTime)
{
    // 1. 更新仿真引擎（物理计算）
    g_simulator->update(deltaTime);

    // 2. 获取当前系统数据
    SystemData data = g_simulator->getLatestData();

    // 3. 检测告警条件
    AlertLevel highestLevel = g_alertManager->checkCondition(data);

    // 红色告警强制停车逻辑
    // 仅在 DANGER (危险) 级别时强制停车，WARNING (警告) 级别不停车
    // 如果是手动注入故障，必须等待物理参数真正达到故障目标值后才停车
    bool shouldStop = false;
    if (highestLevel == AlertLevel::DANGER)
    {
        if (g_simulator->isFaultActive())
        {
            // 如果有故障注入，检查是否已达到目标
            if (g_simulator->isFaultTargetReached())
            {
                shouldStop = true;
            }
        }
        else
        {
            // 如果是自然发生的故障，立即停车
            shouldStop = true;
        }
    }

    if (shouldStop)
    {
        // 只有在非停车且非关闭状态下才执行，避免重复触发
        if (!g_simulator->isStopping() && data.systemState != SystemState::OFF)
        {
            std::cout << "\n========================================" << std::endl;
            std::cout << "!!! CRITICAL DANGER DETECTED !!!" << std::endl;
            std::cout << "!!! INITIATING EMERGENCY SHUTDOWN !!!" << std::endl;
            std::cout << "========================================\n"
                      << std::endl;

            g_logger->recordEvent(data.timestamp, "CRITICAL DANGER: EMERGENCY SHUTDOWN INITIATED");
            g_simulator->stopEngine();
        }
    }

    // 4. 更新告警计时器
    g_alertManager->updateTimers(deltaTime);

    // 5. 获取新触发的告警（用于日志记录）
    std::vector<AlertInfo> newAlerts = g_alertManager->getNewAlerts();
    for (const auto &alert : newAlerts)
    {
        g_logger->recordAlert(alert.timestamp, alert);
    }

    // 6. 记录数据到CSV（每5ms记录一次）
    static double dataLogTimer = 0.0;
    dataLogTimer += deltaTime;
    if (dataLogTimer >= 0.005)
    {
        g_logger->recordData(data.timestamp, data);
        dataLogTimer = 0.0;
    }

    // 7. 检查是否需要更新UI（降低频率到30Hz）
    static double uiUpdateTimer = 0.0;
    uiUpdateTimer += deltaTime;

    if (uiUpdateTimer >= UI_UPDATE_INTERVAL)
    {
        updateUI();
        uiUpdateTimer = 0.0;
    }
}

void updateUI()
{
    // 获取系统数据
    SystemData data = g_simulator->getLatestData();

    // 获取活跃告警消息
    std::vector<std::string> alerts = g_alertManager->getActiveMessages();

    // 更新界面
    g_ui->update(data, alerts);
}

void preciseSleep(double milliseconds)
{
    if (milliseconds <= 0)
        return;

    auto start = std::chrono::high_resolution_clock::now();
    auto targetDuration = std::chrono::duration<double, std::milli>(milliseconds);

    while (true)
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = now - start;

        if (elapsed >= targetDuration)
        {
            break;
        }

        // 如果剩余时间超过1ms，使用sleep_for
        auto remaining = targetDuration - elapsed;
        if (remaining > std::chrono::milliseconds(1))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        // 否则使用自旋等待（更精确）
    }
}

double getCurrentTime()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - startTime;
    return elapsed.count();
}
