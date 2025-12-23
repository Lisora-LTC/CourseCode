#include "EngineUI.h"
#include <cmath>
#include <sstream>
#include <iomanip>

// 注意：这里使用的是通用的绘制接口
// 实际实现时需要根据选择的图形库（EasyX/Qt/OpenGL）进行适配

// ==================== 构造与析构 ====================

EngineUI::EngineUI(int width, int height)
    : windowWidth_(width),
      windowHeight_(height),
      initialized_(false),
      shouldClose_(false),
      startLightOn_(false),
      runLightOn_(false)
{
    // 初始化按钮位置信息
}

EngineUI::~EngineUI()
{
    shutdown();
}

// ==================== 初始化与清理 ====================

bool EngineUI::initialize()
{
    // TODO: 初始化图形库
    // 例如：initgraph(windowWidth_, windowHeight_);
    // 设置绘图模式、字体等

    initialized_ = true;
    return true;
}

void EngineUI::shutdown()
{
    // TODO: 清理图形资源
    // 例如：closegraph();

    initialized_ = false;
}

// ==================== 核心绘制函数 ====================

void EngineUI::update(const SystemData &data, const std::vector<std::string> &alerts)
{
    // TODO: 实现完整界面更新
    // 1. 清空画面
    // 2. 绘制背景
    // 3. 绘制左发动机表盘
    // 4. 绘制右发动机表盘
    // 5. 绘制燃油流速显示
    // 6. 绘制状态指示器
    // 7. 绘制按钮
    // 8. 绘制告警消息
    // 9. 刷新显示
}

void EngineUI::clear()
{
    // TODO: 清空画面
    // 使用黑色或深灰色填充背景
}

void EngineUI::present()
{
    // TODO: 刷新显示
    // 将后台缓冲区内容绘制到屏幕
    // 例如：FlushBatchDraw();
}

// ==================== 表盘绘制函数 ====================

void EngineUI::drawGauge(double value, double minVal, double maxVal,
                         AlertLevel level, Point pos, double radius,
                         const std::string &label)
{
    // TODO: 实现通用表盘绘制
    // 1. 绘制表盘外圈（圆形或弧形）
    // 2. 绘制刻度线
    // 3. 根据value计算角度（0-210度）
    // 4. 绘制扇形指示器
    // 5. 根据level设置颜色
    // 6. 绘制中心数字显示
    // 7. 绘制标签文字
    // 8. 如果level == INVALID，显示"--"
}

void EngineUI::drawN1Gauge(const EngineData &engine, AlertLevel level, Point pos)
{
    // TODO: 实现N1转速表盘绘制
    // 调用drawGauge，传入N1相关参数
    // minVal = 0, maxVal = 125
}

void EngineUI::drawEGTGauge(const EngineData &engine, AlertLevel level, Point pos)
{
    // TODO: 实现EGT温度表盘绘制
    // 调用drawGauge，传入EGT相关参数
    // minVal = 0, maxVal = 1200
}

// ==================== 数字显示函数 ====================

void EngineUI::drawDigitalDisplay(double value, AlertLevel level, Point pos,
                                  const std::string &label, const std::string &unit,
                                  int precision)
{
    // TODO: 实现数字显示
    // 1. 获取对应告警级别的颜色
    // 2. 绘制标签文字
    // 3. 格式化数值（保留指定小数位）
    // 4. 如果level == INVALID，显示"--"
    // 5. 绘制数值和单位
}

void EngineUI::drawFuelFlowDisplay(const FuelData &fuelData, AlertLevel level, Point pos)
{
    // TODO: 实现燃油流速显示
    // 调用drawDigitalDisplay
    // 显示燃油余量和流速
}

// ==================== 状态显示函数 ====================

void EngineUI::drawStatusIndicators(const SystemData &data, Point pos)
{
    // TODO: 实现状态指示器绘制
    // 1. 绘制START灯（圆形或矩形）
    // 2. 根据startLightOn_设置亮度/颜色
    // 3. 绘制RUN灯
    // 4. 根据runLightOn_设置亮度/颜色
    // 5. 绘制标签文字
}

void EngineUI::updateIndicators(const SystemData &data)
{
    // TODO: 实现指示器状态更新
    // 1. START灯：启动阶段（STARTING_P1或STARTING_P2）亮起
    // 2. RUN灯：稳态阶段（RUNNING且N1 >= 95%）亮起
    // 3. 如果N1 < 95%，RUN灯熄灭
}

// ==================== 告警显示函数 ====================

void EngineUI::drawCASMessages(const std::vector<std::string> &messages, Point pos)
{
    // TODO: 实现告警消息绘制
    // 1. 在指定位置开始
    // 2. 逐行绘制告警消息
    // 3. 根据消息中的关键词判断颜色
    //    - 包含"WARNING"：红色
    //    - 包含"CAUTION"：琥珀色
    //    - 其他：白色
    // 4. 每行间隔适当距离
}

// ==================== 按钮绘制与交互 ====================

void EngineUI::drawButton(ButtonID id, Point pos, double width, double height,
                          const std::string &label, bool enabled)
{
    // TODO: 实现按钮绘制
    // 1. 绘制矩形边框
    // 2. 如果enabled，使用正常颜色；否则使用灰色
    // 3. 绘制按钮文字（居中）
    // 4. 记录按钮位置信息（用于点击检测）
}

void EngineUI::drawAllButtons()
{
    // TODO: 实现所有按钮绘制
    // 计算按钮位置并调用drawButton
    // - START按钮
    // - STOP按钮
    // - INCREASE THRUST按钮
    // - DECREASE THRUST按钮
}

ButtonID *EngineUI::checkButtonClick(int x, int y)
{
    // TODO: 实现按钮点击检测
    // 1. 遍历buttons_列表
    // 2. 检查点(x, y)是否在某个按钮的范围内
    // 3. 如果在，返回该按钮的ID指针
    // 4. 如果不在任何按钮内，返回nullptr

    return nullptr; // 占位返回
}

void EngineUI::setButtonCallback(std::function<void(ButtonID)> callback)
{
    buttonCallback_ = callback;
}

void EngineUI::onButtonClicked(ButtonID id)
{
    if (buttonCallback_)
    {
        buttonCallback_(id);
    }
}

// ==================== 事件处理 ====================

bool EngineUI::processEvents()
{
    // TODO: 实现事件处理
    // 1. 检查是否有输入事件
    // 2. 处理鼠标点击
    //    - 获取鼠标位置
    //    - 调用checkButtonClick
    //    - 如果点击了按钮，调用onButtonClicked
    // 3. 处理键盘输入（如ESC退出）
    // 4. 检查窗口关闭事件

    return !shouldClose_;
}

bool EngineUI::shouldClose() const
{
    return shouldClose_;
}

// ==================== 私有辅助函数 ====================

Color EngineUI::getColorForLevel(AlertLevel level) const
{
    switch (level)
    {
    case AlertLevel::NORMAL:
    case AlertLevel::ADVISORY:
        return Color::White();
    case AlertLevel::CAUTION:
        return Color::Amber();
    case AlertLevel::WARNING:
        return Color::Red();
    case AlertLevel::INVALID:
        return Color::Gray();
    default:
        return Color::White();
    }
}

void EngineUI::drawArc(Point center, double radius, double startAngle,
                       double endAngle, Color color)
{
    // TODO: 实现扇形绘制
    // 使用图形库的arc或类似函数
    // 注意角度单位转换（度 -> 弧度）
}

void EngineUI::drawText(const std::string &text, Point pos, Color color, int fontSize)
{
    // TODO: 实现文字绘制
    // 设置字体大小和颜色
    // 在指定位置绘制文字
}

void EngineUI::drawRect(Point pos, double width, double height, Color color, bool filled)
{
    // TODO: 实现矩形绘制
    // 根据filled参数决定是填充还是只绘制边框
}

void EngineUI::drawCircle(Point center, double radius, Color color, bool filled)
{
    // TODO: 实现圆形绘制
    // 根据filled参数决定是填充还是只绘制边框
}

void EngineUI::drawLine(Point start, Point end, Color color, double thickness)
{
    // TODO: 实现直线绘制
    // 设置线宽和颜色
}

double EngineUI::mapValueToAngle(double value, double minVal, double maxVal) const
{
    // 将数值映射到0-210度范围
    if (maxVal <= minVal)
        return 0.0;

    double normalized = (value - minVal) / (maxVal - minVal);
    normalized = std::max(0.0, std::min(1.0, normalized)); // 限制在[0, 1]

    return normalized * Constants::GAUGE_ANGLE_MAX;
}

bool EngineUI::pointInRect(int px, int py, Point rect, double width, double height) const
{
    return px >= rect.x && px <= rect.x + width &&
           py >= rect.y && py <= rect.y + height;
}
