#include "EngineUI.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <graphics.h> // EasyX图形库
#include <conio.h>

// 注意：使用EasyX图形库实现
// EasyX是Windows平台的简单图形库

// UI布局常量
const double PI = 3.14159265358979323846;
const int GAUGE_RADIUS = 80;  // 表盘半径
const int BUTTON_WIDTH = 100; // 按钮宽度
const int BUTTON_HEIGHT = 40; // 按钮高度

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
    // 初始化图形窗口
    initgraph(windowWidth_, windowHeight_);

    // 设置窗口标题
    SetWindowText(GetHWnd(), L"EICAS - Engine Monitoring System");

    // 设置绘图模式
    setbkmode(TRANSPARENT); // 文字背景透明
    setbkcolor(BLACK);      // 背景色黑色
    cleardevice();          // 清屏

    // 开启批量绘图模式（提高性能）
    BeginBatchDraw();

    // 初始化按钮位置
    // START按钮 - 左下
    ButtonInfo startBtn;
    startBtn.id = ButtonID::START;
    startBtn.pos = Point(50, windowHeight_ - 80);
    startBtn.width = BUTTON_WIDTH;
    startBtn.height = BUTTON_HEIGHT;
    buttons_.push_back(startBtn);

    // STOP按钮 - START右侧
    ButtonInfo stopBtn;
    stopBtn.id = ButtonID::STOP;
    stopBtn.pos = Point(170, windowHeight_ - 80);
    stopBtn.width = BUTTON_WIDTH;
    stopBtn.height = BUTTON_HEIGHT;
    buttons_.push_back(stopBtn);

    // 增加推力按钮 - 中间位置
    ButtonInfo incBtn;
    incBtn.id = ButtonID::INCREASE_THRUST;
    incBtn.pos = Point(windowWidth_ / 2 - 60, windowHeight_ - 80);
    incBtn.width = 120;
    incBtn.height = BUTTON_HEIGHT;
    buttons_.push_back(incBtn);

    // 减小推力按钮 - 增加推力按钮右侧
    ButtonInfo decBtn;
    decBtn.id = ButtonID::DECREASE_THRUST;
    decBtn.pos = Point(windowWidth_ / 2 + 80, windowHeight_ - 80);
    decBtn.width = 120;
    decBtn.height = BUTTON_HEIGHT;
    buttons_.push_back(decBtn);

    initialized_ = true;
    return true;
}

void EngineUI::shutdown()
{
    // 结束批量绘图
    EndBatchDraw();

    // 关闭图形窗口
    closegraph();

    initialized_ = false;
}

// ==================== 核心绘制函数 ====================

void EngineUI::update(const SystemData &data, const std::vector<std::string> &alerts)
{
    // 1. 清空画面
    clear();

    // 2. 绘制标题
    drawText("ENGINE INDICATION AND CREW ALERTING SYSTEM", Point(windowWidth_ / 2 - 200, 20), Color::White(), 20);

    // 3. 绘制左发动机表盘区域
    int leftX = windowWidth_ / 4;
    int gaugeY = 200;

    // 计算左发告警级别
    AlertLevel leftN1Level = calculateN1AlertLevel(data.leftEngine);
    AlertLevel leftEGTLevel = calculateEGTAlertLevel(data.leftEngine);

    // 左发N1表盘
    drawN1Gauge(data.leftEngine, leftN1Level, Point(leftX, gaugeY));

    // 左发EGT表盘
    drawEGTGauge(data.leftEngine, leftEGTLevel, Point(leftX, gaugeY + 200));

    // 4. 绘制右发动机表盘区域
    int rightX = windowWidth_ * 3 / 4;

    // 计算右发告警级别
    AlertLevel rightN1Level = calculateN1AlertLevel(data.rightEngine);
    AlertLevel rightEGTLevel = calculateEGTAlertLevel(data.rightEngine);

    // 右发N1表盘
    drawN1Gauge(data.rightEngine, rightN1Level, Point(rightX, gaugeY));

    // 右发EGT表盘
    drawEGTGauge(data.rightEngine, rightEGTLevel, Point(rightX, gaugeY + 200));

    // 5. 绘制燃油流速显示（中间位置）
    drawFuelFlowDisplay(data.fuel, AlertLevel::NORMAL, Point(windowWidth_ / 2, gaugeY + 100));

    // 6. 绘制状态指示器
    drawStatusIndicators(data, Point(windowWidth_ / 2 - 100, 100));

    // 7. 绘制按钮
    drawAllButtons();

    // 8. 绘制告警消息（右侧CAS区域）
    int casX = windowWidth_ - 320;
    int casY = 80;

    // 绘制CAS区域边框
    setlinecolor(RGB(100, 100, 100));
    setlinestyle(PS_SOLID, 2);
    rectangle(casX - 10, casY - 10, windowWidth_ - 20, casY + 450);

    // 绘制CAS标题
    settextcolor(RGB(200, 200, 200));
    settextstyle(20, 0, L"Arial Bold");
    outtextxy(casX, casY, L"CAS MESSAGES");

    // 绘制分隔线
    setlinecolor(RGB(100, 100, 100));
    line(casX - 10, casY + 30, windowWidth_ - 20, casY + 30);

    if (!alerts.empty())
    {
        drawCASMessages(alerts, Point(casX, casY + 45));
    }
    else
    {
        // 无告警时显示
        settextcolor(RGB(0, 200, 0));
        settextstyle(16, 0, L"Arial");
        outtextxy(casX + 20, casY + 60, L"NO ALERTS");
    }

    // 9. 刷新显示
    present();
}

void EngineUI::clear()
{
    // 使用黑色填充背景
    setbkcolor(BLACK);
    cleardevice();
}

void EngineUI::present()
{
    // 刷新显示缓冲区
    FlushBatchDraw();
}

// ==================== 表盘绘制函数 ====================

void EngineUI::drawGauge(double value, double minVal, double maxVal,
                         AlertLevel level, Point pos, double radius,
                         const std::string &label)
{
    Color gaugeColor = getColorForLevel(level);

    // 1. 绘制表盘外圈（圆圈）
    setlinecolor(RGB(80, 80, 80));
    setlinestyle(PS_SOLID, 2);
    circle((int)pos.x, (int)pos.y, (int)radius);

    // 2. 绘制底部灰色弧线（150°-360°，即210°范围）
    setlinecolor(RGB(60, 60, 60));
    setlinestyle(PS_SOLID, 3);
    drawArc(pos, radius - 5, 150, 360, Color::DarkGray());

    // 3. 绘制刻度线（0%, 25%, 50%, 75%, 100%）
    for (int i = 0; i <= 4; i++)
    {
        double percent = i * 0.25;
        double angle = 150 + percent * 210; // 150°到360°
        double rad = angle * PI / 180.0;

        int x1 = (int)(pos.x + (radius - 10) * cos(rad));
        int y1 = (int)(pos.y - (radius - 10) * sin(rad));
        int x2 = (int)(pos.x + (radius - 2) * cos(rad));
        int y2 = (int)(pos.y - (radius - 2) * sin(rad));

        setlinecolor(RGB(100, 100, 100));
        line(x1, y1, x2, y2);
    }

    // 4. 绘制扇形指示器（从150度开始）
    if (level != AlertLevel::INVALID)
    {
        double angle = mapValueToAngle(value, minVal, maxVal);

        // 绘制填充扇形
        setfillcolor(RGB(gaugeColor.r / 3, gaugeColor.g / 3, gaugeColor.b / 3));
        setlinecolor(RGB(gaugeColor.r, gaugeColor.g, gaugeColor.b));
        setlinestyle(PS_SOLID, 3);

        // 使用多边形绘制填充扇形
        const int segments = 40;
        POINT points[42];                     // 多2个：中心点和闭合点
        points[0] = {(int)pos.x, (int)pos.y}; // 中心点

        for (int i = 0; i <= segments; i++)
        {
            double currentAngle = 150 + angle * i / segments;
            double rad = currentAngle * PI / 180.0;
            points[i + 1].x = (int)(pos.x + (radius - 5) * cos(rad));
            points[i + 1].y = (int)(pos.y - (radius - 5) * sin(rad));
        }

        fillpolygon(points, segments + 2);

        // 绘制边框弧线
        setlinecolor(RGB(gaugeColor.r, gaugeColor.g, gaugeColor.b));
        setlinestyle(PS_SOLID, 4);
        drawArc(pos, radius - 5, 150, 150 + angle, gaugeColor);

        // 绘制指针
        double endAngle = (150 + angle) * PI / 180.0;
        int needleX = (int)(pos.x + (radius - 15) * cos(endAngle));
        int needleY = (int)(pos.y - (radius - 15) * sin(endAngle));

        setlinecolor(RGB(gaugeColor.r, gaugeColor.g, gaugeColor.b));
        setlinestyle(PS_SOLID, 3);
        line((int)pos.x, (int)pos.y, needleX, needleY);

        // 绘制中心圆点
        setfillcolor(RGB(gaugeColor.r, gaugeColor.g, gaugeColor.b));
        fillcircle((int)pos.x, (int)pos.y, 5);
    }

    // 5. 绘制中心数字显示
    std::ostringstream oss;
    if (level == AlertLevel::INVALID)
    {
        oss << "--";
    }
    else
    {
        oss << std::fixed << std::setprecision(1) << value;
    }

    settextcolor(RGB(gaugeColor.r, gaugeColor.g, gaugeColor.b));
    settextstyle(28, 0, L"Arial");

    std::wstring wvalue(oss.str().begin(), oss.str().end());
    int textWidth = textwidth(wvalue.c_str());
    int textHeight = textheight(wvalue.c_str());
    outtextxy((int)(pos.x - textWidth / 2), (int)(pos.y + 20), wvalue.c_str());

    // 6. 绘制标签文字
    settextcolor(WHITE);
    settextstyle(18, 0, L"Arial");
    std::wstring wlabel(label.begin(), label.end());
    textWidth = textwidth(wlabel.c_str());
    outtextxy((int)(pos.x - textWidth / 2), (int)(pos.y + radius + 15), wlabel.c_str());
}

void EngineUI::drawN1Gauge(const EngineData &engine, AlertLevel level, Point pos)
{
    // 检查是否为无效值
    double value = engine.n1Percentage;
    AlertLevel displayLevel = level;

    // 如果值为-9999或其他特殊标记，设置为INVALID
    if (value < -9000 || value > 200)
    {
        displayLevel = AlertLevel::INVALID;
        value = 0;
    }

    // 绘制N1转速表盘
    std::string engineLabel = (engine.engineID == EngineID::LEFT) ? "L N1" : "R N1";
    drawGauge(value, 0, 125, displayLevel, pos, GAUGE_RADIUS, engineLabel);
}

void EngineUI::drawEGTGauge(const EngineData &engine, AlertLevel level, Point pos)
{
    // 检查是否为无效值
    double value = engine.egtTemperature;
    AlertLevel displayLevel = level;

    // 如果值为-9999或其他特殊标记，设置为INVALID
    if (value < -9000 || value > 2000)
    {
        displayLevel = AlertLevel::INVALID;
        value = 0;
    }

    // 绘制EGT温度表盘
    std::string engineLabel = (engine.engineID == EngineID::LEFT) ? "L EGT" : "R EGT";
    drawGauge(value, 0, 1200, displayLevel, pos, GAUGE_RADIUS, engineLabel);
}

// ==================== 数字显示函数 ====================

void EngineUI::drawDigitalDisplay(double value, AlertLevel level, Point pos,
                                  const std::string &label, const std::string &unit,
                                  int precision)
{
    Color color = getColorForLevel(level);

    // 绘制标签
    settextcolor(WHITE);
    settextstyle(14, 0, L"Arial");
    std::wstring wlabel(label.begin(), label.end());
    outtextxy((int)pos.x, (int)pos.y, wlabel.c_str());

    // 格式化数值
    std::ostringstream oss;
    if (level == AlertLevel::INVALID)
    {
        oss << "--";
    }
    else
    {
        oss << std::fixed << std::setprecision(precision) << value;
    }
    oss << " " << unit;

    // 绘制数值
    settextcolor(RGB(color.r, color.g, color.b));
    settextstyle(18, 0, L"Arial");
    std::wstring wvalue(oss.str().begin(), oss.str().end());
    outtextxy((int)pos.x + 60, (int)pos.y - 2, wvalue.c_str());
}

void EngineUI::drawFuelFlowDisplay(const FuelData &fuelData, AlertLevel level, Point pos)
{
    // 显示燃油余量
    drawDigitalDisplay(fuelData.capacity, level, Point(pos.x, pos.y), "FUEL", "units", 0);

    // 显示燃油流速
    drawDigitalDisplay(fuelData.flowRate, level, Point(pos.x, pos.y + 40), "FLOW", "u/s", 1);
}

// ==================== 状态显示函数 ====================

void EngineUI::drawStatusIndicators(const SystemData &data, Point pos)
{
    // 更新指示器状态
    updateIndicators(data);

    // 绘制START指示器
    Color startColor = startLightOn_ ? Color::White() : Color::DarkGray();
    setfillcolor(RGB(startColor.r, startColor.g, startColor.b));
    fillcircle((int)pos.x, (int)pos.y, 15);

    settextcolor(WHITE);
    settextstyle(14, 0, L"Arial");
    outtextxy((int)pos.x + 25, (int)pos.y - 7, L"START");

    // 绘制RUN指示器
    Color runColor = runLightOn_ ? Color::White() : Color::DarkGray();
    setfillcolor(RGB(runColor.r, runColor.g, runColor.b));
    fillcircle((int)pos.x + 150, (int)pos.y, 15);

    outtextxy((int)pos.x + 175, (int)pos.y - 7, L"RUN");
}

void EngineUI::updateIndicators(const SystemData &data)
{
    // START灯：启动阶段亮起
    startLightOn_ = (data.leftEngine.state == SystemState::STARTING_P1 ||
                     data.leftEngine.state == SystemState::STARTING_P2 ||
                     data.rightEngine.state == SystemState::STARTING_P1 ||
                     data.rightEngine.state == SystemState::STARTING_P2);

    // RUN灯：稳态运行且N1 >= 95%时亮起
    runLightOn_ = ((data.leftEngine.state == SystemState::RUNNING &&
                    data.leftEngine.n1Percentage >= 95.0) ||
                   (data.rightEngine.state == SystemState::RUNNING &&
                    data.rightEngine.n1Percentage >= 95.0));
}

// ==================== 告警显示函数 ====================

void EngineUI::drawCASMessages(const std::vector<std::string> &messages, Point pos)
{
    int yOffset = 0;
    const int lineHeight = 25;

    settextstyle(14, 0, L"Arial");

    for (const auto &msg : messages)
    {
        // 根据关键词确定颜色
        Color msgColor = Color::White();
        if (msg.find("WARNING") != std::string::npos)
        {
            msgColor = Color::Red();
        }
        else if (msg.find("CAUTION") != std::string::npos)
        {
            msgColor = Color::Amber();
        }

        settextcolor(RGB(msgColor.r, msgColor.g, msgColor.b));
        std::wstring wmsg(msg.begin(), msg.end());
        outtextxy((int)pos.x, (int)pos.y + yOffset, wmsg.c_str());

        yOffset += lineHeight;
    }
}

// ==================== 按钮绘制与交互 ====================

void EngineUI::drawButton(ButtonID id, Point pos, double width, double height,
                          const std::string &label, bool enabled)
{
    // 绘制矩形边框
    Color btnColor = enabled ? Color::White() : Color::Gray();
    setlinecolor(RGB(btnColor.r, btnColor.g, btnColor.b));
    setlinestyle(PS_SOLID, 2);
    rectangle((int)pos.x, (int)pos.y, (int)(pos.x + width), (int)(pos.y + height));

    // 绘制按钮文字（居中）
    settextcolor(RGB(btnColor.r, btnColor.g, btnColor.b));
    settextstyle(14, 0, L"Arial");

    std::wstring wlabel(label.begin(), label.end());
    int textWidth = textwidth(wlabel.c_str());
    int textHeight = textheight(wlabel.c_str());

    outtextxy((int)(pos.x + width / 2 - textWidth / 2),
              (int)(pos.y + height / 2 - textHeight / 2),
              wlabel.c_str());
}

void EngineUI::drawAllButtons()
{
    for (const auto &btn : buttons_)
    {
        std::string label;
        switch (btn.id)
        {
        case ButtonID::START:
            label = "START";
            break;
        case ButtonID::STOP:
            label = "STOP";
            break;
        case ButtonID::INCREASE_THRUST:
            label = "INCR THRUST";
            break;
        case ButtonID::DECREASE_THRUST:
            label = "DECR THRUST";
            break;
        }
        drawButton(btn.id, btn.pos, btn.width, btn.height, label, true);
    }
}

ButtonID *EngineUI::checkButtonClick(int x, int y)
{
    for (auto &btn : buttons_)
    {
        if (pointInRect(x, y, btn.pos, btn.width, btn.height))
        {
            return &btn.id;
        }
    }
    return nullptr;
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
    // 检查鼠标点击事件
    if (MouseHit())
    {
        MOUSEMSG msg = GetMouseMsg();

        if (msg.uMsg == WM_LBUTTONDOWN)
        {
            // 检查是否点击了按钮
            ButtonID *clickedBtn = checkButtonClick(msg.x, msg.y);
            if (clickedBtn != nullptr)
            {
                onButtonClicked(*clickedBtn);
            }
        }
    }

    // 检查键盘输入（ESC退出）
    if (_kbhit())
    {
        int key = _getch();
        if (key == 27)
        { // ESC键
            shouldClose_ = true;
        }
    }

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
    // EasyX中的arc函数使用弧度，需要转换
    setlinecolor(RGB(color.r, color.g, color.b));

    // 将角度转换为弧度，并调整坐标系（EasyX需要矩形边界框）
    double startRad = startAngle * PI / 180.0;
    double endRad = endAngle * PI / 180.0;

    // 使用多条线段绘制弧线
    const int segments = 50;
    for (int i = 0; i < segments; i++)
    {
        double angle1 = startRad + (endRad - startRad) * i / segments;
        double angle2 = startRad + (endRad - startRad) * (i + 1) / segments;

        int x1 = (int)(center.x + radius * cos(angle1));
        int y1 = (int)(center.y - radius * sin(angle1));
        int x2 = (int)(center.x + radius * cos(angle2));
        int y2 = (int)(center.y - radius * sin(angle2));

        line(x1, y1, x2, y2);
    }
}

void EngineUI::drawText(const std::string &text, Point pos, Color color, int fontSize)
{
    settextcolor(RGB(color.r, color.g, color.b));
    settextstyle(fontSize, 0, L"Arial");

    std::wstring wtext(text.begin(), text.end());
    outtextxy((int)pos.x, (int)pos.y, wtext.c_str());
}

void EngineUI::drawRect(Point pos, double width, double height, Color color, bool filled)
{
    if (filled)
    {
        setfillcolor(RGB(color.r, color.g, color.b));
        fillrectangle((int)pos.x, (int)pos.y, (int)(pos.x + width), (int)(pos.y + height));
    }
    else
    {
        setlinecolor(RGB(color.r, color.g, color.b));
        rectangle((int)pos.x, (int)pos.y, (int)(pos.x + width), (int)(pos.y + height));
    }
}

void EngineUI::drawCircle(Point center, double radius, Color color, bool filled)
{
    if (filled)
    {
        setfillcolor(RGB(color.r, color.g, color.b));
        fillcircle((int)center.x, (int)center.y, (int)radius);
    }
    else
    {
        setlinecolor(RGB(color.r, color.g, color.b));
        circle((int)center.x, (int)center.y, (int)radius);
    }
}

// ==================== 告警级别计算函数 ====================

AlertLevel EngineUI::calculateN1AlertLevel(const EngineData &engine) const
{
    // 检查传感器有效性
    if (!engine.n1SensorValid)
    {
        return AlertLevel::INVALID;
    }

    double n1 = engine.n1Percentage;

    // 超转2级（>120%）- 危险
    if (n1 > 120.0)
    {
        return AlertLevel::DANGER;
    }

    // 超转1级（>105%）- 警告
    if (n1 > 105.0)
    {
        return AlertLevel::WARNING;
    }

    // 运行中转速过低（<30%）- 注意
    if (engine.state == SystemState::RUNNING && n1 < 30.0)
    {
        return AlertLevel::CAUTION;
    }

    // 正常
    return AlertLevel::NORMAL;
}

AlertLevel EngineUI::calculateEGTAlertLevel(const EngineData &engine) const
{
    // 检查传感器有效性
    if (!engine.egtSensorValid)
    {
        return AlertLevel::INVALID;
    }

    double egt = engine.egtTemperature;
    bool isStarting = (engine.state == SystemState::STARTING_P1 ||
                       engine.state == SystemState::STARTING_P2);
    bool isRunning = (engine.state == SystemState::RUNNING);

    if (isStarting)
    {
        // 启动阶段：临时允许950°C
        if (egt > 1000.0)
        {
            return AlertLevel::DANGER; // 超温2
        }
        if (egt > 950.0)
        {
            return AlertLevel::WARNING; // 超温1
        }
    }
    else if (isRunning)
    {
        // 运行阶段：最高850°C
        if (egt > 900.0)
        {
            return AlertLevel::DANGER; // 超温4
        }
        if (egt > 850.0)
        {
            return AlertLevel::WARNING; // 超温3
        }
        if (egt < 400.0)
        {
            return AlertLevel::CAUTION; // 温度过低
        }
    }

    // 正常
    return AlertLevel::NORMAL;
}

void EngineUI::drawLine(Point start, Point end, Color color, double thickness)
{
    setlinecolor(RGB(color.r, color.g, color.b));
    setlinestyle(PS_SOLID, (int)thickness);
    line((int)start.x, (int)start.y, (int)end.x, (int)end.y);
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
