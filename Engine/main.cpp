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
    g_ui = new EngineUI(1280, 720);
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
        std::cout << "[" << g_simulator->getElapsedTime() << "s] Button: START" << std::endl;
        g_simulator->startEngine();
        break;

    case ButtonID::STOP:
        std::cout << "[" << g_simulator->getElapsedTime() << "s] Button: STOP (Priority)" << std::endl;
        g_simulator->stopEngine();
        break;

    case ButtonID::INCREASE_THRUST:
        std::cout << "[" << g_simulator->getElapsedTime() << "s] Button: INCREASE THRUST" << std::endl;
        g_simulator->adjustThrust(+1);
        break;

    case ButtonID::DECREASE_THRUST:
        std::cout << "[" << g_simulator->getElapsedTime() << "s] Button: DECREASE THRUST" << std::endl;
        g_simulator->adjustThrust(-1);
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

    // 4. 更新告警计时器
    g_alertManager->updateTimers(deltaTime);

    // 5. 获取新触发的告警（用于日志记录）
    std::vector<AlertInfo> newAlerts = g_alertManager->getNewAlerts();
    for (const auto &alert : newAlerts)
    {
        g_logger->recordEvent(alert.timestamp, alert.faultType, alert.message);
    }

    // 6. 记录数据到CSV（每1秒记录一次）
    static double dataLogTimer = 0.0;
    dataLogTimer += deltaTime;
    if (dataLogTimer >= 1.0)
    {
        g_logger->recordData(data.timestamp, data.leftEngine, data.rightEngine,
                             data.fuelData, highestLevel);
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
