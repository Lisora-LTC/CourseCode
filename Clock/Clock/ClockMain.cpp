
#include <graphics.h>   // EasyX 图形库
#include <conio.h>      // _kbhit(), _getch()
#include <ctime>        // }};/在 updateThemeAndHands 中更新）在 updateThemeAndHands 中更新）ands 中更新） updateThemeAndHands 中更新）始颜色会在 updateThemeAndHands 中更新）局指针对象（初始颜色会在updateTheme中更新）在updateTheme中更新）include <cmath>        // sin(), cos() 用于计算指针坐标
#include <Windows.h>    // Sleep() 函数
#include <algorithm>    // min, max 函数
#include "AntiAlias.h"  // 抗锯齿算法头文件
#include "ThemeManager.h"  // 主题管理模块

// 全局中心点和分辨率参数
#define WIN_W 1280
#define WIN_H 960
const int CENTER_X = WIN_W / 2;
const int CENTER_Y = 400;  // 向上移动，给下方文字留出空间（原来是 WIN_H / 2 = 480）

// ==================== 抗锯齿算法选择 ====================
// 设置为 1 使用 2xSSAA，0 使用 Wu's 算法
// 2xSSAA 提供更好的质量但性能开销更大
// 注意：2xSSAA在1280x960分辨率下会很慢，建议只对指针使用
#define USE_2X_SSAA 1
#define USE_2X_SSAA_FOR_STATIC 0  // 静态元素（刻度、边框）是否使用2xSSAA
// =======================================================

// 抗锯齿绘制辅助宏（自动根据USE_2X_SSAA选择算法）
// 用于动态元素（指针）
#if USE_2X_SSAA
    #define AA_DrawLine(x1, y1, x2, y2, color, thickness) \
        AntiAlias::SSAA::drawLine2xSSAA(x1, y1, x2, y2, color, thickness, WIN_W, WIN_H)
    #define AA_DrawCircle(cx, cy, r, color, filled) \
        AntiAlias::SSAA::drawCircle2xSSAA(cx, cy, r, color, filled, WIN_W, WIN_H)
#else
    #define AA_DrawLine(x1, y1, x2, y2, color, thickness) \
        AntiAlias::drawAntiAliasedLine(x1, y1, x2, y2, color, thickness, WIN_W, WIN_H)
    #define AA_DrawCircle(cx, cy, r, color, filled) \
        AntiAlias::drawAntiAliasedCircle(cx, cy, r, color, filled, WIN_W, WIN_H)
#endif

// 用于静态元素（刻度、边框等）
#if USE_2X_SSAA_FOR_STATIC
    #define AA_DrawLine_Static(x1, y1, x2, y2, color, thickness) \
        AntiAlias::SSAA::drawLine2xSSAA(x1, y1, x2, y2, color, thickness, WIN_W, WIN_H)
    #define AA_DrawCircle_Static(cx, cy, r, color, filled) \
        AntiAlias::SSAA::drawCircle2xSSAA(cx, cy, r, color, filled, WIN_W, WIN_H)
#else
    #define AA_DrawLine_Static(x1, y1, x2, y2, color, thickness) \
        AntiAlias::drawAntiAliasedLine(x1, y1, x2, y2, color, thickness, WIN_W, WIN_H)
    #define AA_DrawCircle_Static(cx, cy, r, color, filled) \
        AntiAlias::drawAntiAliasedCircle(cx, cy, r, color, filled, WIN_W, WIN_H)
#endif

// 时钟指针类
class ClockHand {
private:
    int centerX;        // 指针中心X坐标
    int centerY;        // 指针中心Y坐标
    double length;      // 指针长度
    double angle;       // 指针角度(弧度)，0度为12点方向
    COLORREF color;     // 指针颜色
    int thickness;      // 指针粗细

public:
    // 构造函数
    ClockHand(int cx = CENTER_X, int cy = CENTER_Y, double len = 100, 
              double ang = 0, COLORREF col = WHITE, int thick = 2)
        : centerX(cx), centerY(cy), length(len), angle(ang), 
          color(col), thickness(thick) {}

    // 设置指针属性
    void setCenter(int x, int y) { centerX = x; centerY = y; }
    void setLength(double len) { length = len; }
    void setAngle(double ang) { angle = ang; }
    void setColor(COLORREF col) { color = col; }
    void setThickness(int thick) { thickness = thick; }

    // 获取指针属性
    double getAngle() const { return angle; }
    double getLength() const { return length; }
    COLORREF getColor() const { return color; }

    // 根据时间设置角度（时分秒针专用方法）
    void setTimeAngle(int value, int maxValue) {
        // 将时间值转换为角度，12点方向为0度，顺时针为正
        angle = (value * 2.0 * 3.14159265359 / maxValue) - 3.14159265359 / 2.0;
    }

    // 渲染指针 - 真抗锯齿版本
    void render() const {
        // 计算指针终点坐标
        int endX = centerX + (int)(length * cos(angle));
        int endY = centerY + (int)(length * sin(angle));

        // 先绘制白色描边增强对比度（使用抗锯齿）
        if (thickness >= 3) {
            AA_DrawLine(centerX, centerY, endX, endY, RGB(255, 255, 255), thickness + 2);
        }

        // 绘制主指针线条（使用抗锯齿）
        AA_DrawLine(centerX, centerY, endX, endY, color, thickness);
        
        // 为指针添加抗锯齿圆润端点
        if (thickness > 1) {
            AA_DrawCircle(endX, endY, 1, color, true);
        }
    }

    // 清除指针（用背景色重绘）
    void clear(COLORREF bgColor = BLACK) const {
        int endX = centerX + (int)(length * cos(angle));
        int endY = centerY + (int)(length * sin(angle));

        setlinecolor(bgColor);
        setlinestyle(PS_SOLID, thickness);
        line(centerX, centerY, endX, endY);
    }
};

// 全局指针对象（初始颜色会在updateTheme中更新）

// ========== 主题模式控制变量 ==========
bool isAutoMode = true;      // 是否为自动模式（根据时间切换）
bool isNightMode = false;    // 当前是否为夜间模式（手动模式下使用）

// 按钮位置和大小
const int BTN_X = WIN_W - 150;  // 按钮左上角X坐标
const int BTN_Y = 20;           // 按钮左上角Y坐标
const int BTN_WIDTH = 120;      // 按钮宽度
const int BTN_HEIGHT = 40;      // 按钮高度
// =========================================

// 全局指针对象（初始颜色会在 updateThemeAndHands 中更新）
ClockHand hourHand(CENTER_X, CENTER_Y, 150, 0, RGB(0,0,0), 10);     // 时针
ClockHand minuteHand(CENTER_X, CENTER_Y, 230, 0, RGB(0,0,0), 6);    // 分针
ClockHand secondHand(CENTER_X, CENTER_Y, 270, 0, RGB(0,0,0), 4);    // 秒针

// 包装函数：更新主题并同步指针颜色
void updateThemeAndHands(int hour) {
    updateTheme(hour);  // 调用 ThemeManager 的函数更新主题
    // 更新指针颜色
    hourHand.setColor(currentTheme.hourHandColor);
    minuteHand.setColor(currentTheme.minuteHandColor);
    secondHand.setColor(currentTheme.secondHandColor);
}

// 手动切换主题模式（不依赖时间）
void switchThemeManually(bool toNightMode) {
    isNightMode = toNightMode;
    currentTheme = isNightMode ? NIGHT_THEME : DAY_THEME;
    // 更新指针颜色
    hourHand.setColor(currentTheme.hourHandColor);
    minuteHand.setColor(currentTheme.minuteHandColor);
    secondHand.setColor(currentTheme.secondHandColor);
}

// 绘制模式切换按钮
void drawModeButton() {
    // 根据主题选择按钮颜色
    COLORREF btnBg = isAutoMode ? RGB(100, 150, 220) : 
                     (isNightMode ? RGB(80, 100, 130) : RGB(150, 180, 220));
    COLORREF btnBorder = isAutoMode ? RGB(80, 120, 200) : 
                         (isNightMode ? RGB(120, 140, 180) : RGB(120, 150, 200));
    COLORREF btnText = RGB(255, 255, 255);
    
    // 绘制按钮背景
    setfillcolor(btnBg);
    setlinecolor(btnBorder);
    setlinestyle(PS_SOLID, 2);
    fillroundrect(BTN_X, BTN_Y, BTN_X + BTN_WIDTH, BTN_Y + BTN_HEIGHT, 10, 10);
    
    // 绘制按钮文字
    settextcolor(btnText);
    settextstyle(18, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);
    
    wchar_t btnTextStr[20];
    if (isAutoMode) {
        wcscpy_s(btnTextStr, L"自动模式");
    } else {
        wcscpy_s(btnTextStr, isNightMode ? L"夜间模式" : L"日间模式");
    }
    
    // 计算文字居中位置
    int textWidth = textwidth(btnTextStr);
    int textHeight = textheight(btnTextStr);
    int textX = BTN_X + (BTN_WIDTH - textWidth) / 2;
    int textY = BTN_Y + (BTN_HEIGHT - textHeight) / 2;
    
    outtextxy(textX, textY, btnTextStr);
}

// 检测按钮点击
bool isButtonClicked(int mouseX, int mouseY) {
    return (mouseX >= BTN_X && mouseX <= BTN_X + BTN_WIDTH &&
            mouseY >= BTN_Y && mouseY <= BTN_Y + BTN_HEIGHT);
}

// 前置声明
void init();            // 初始化时钟界面
void drawClock();       // 绘制时钟表盘
void drawHands(int hour, int minute, int second);  // 绘制时分秒针


// 初始化函数实现 - 使用当前主题配色
void init() {
    const int R_MAIN = 310;
    setbkcolor(currentTheme.bgColor);  // 使用主题背景色
    cleardevice();                     // 清屏
    
    // 绘制背景渐变效果（基于主题背景色的微调）
    for (int r = 360; r > 0; r -= 40) {
        int alpha = 255 - (r * 50 / 360);
        // 提取当前主题背景色的RGB分量
        int bgR = GetRValue(currentTheme.bgColor);
        int bgG = GetGValue(currentTheme.bgColor);
        int bgB = GetBValue(currentTheme.bgColor);
        // 生成略微变亮的渐变
        setfillcolor(RGB(
            min(255, bgR + alpha/20),
            min(255, bgG + alpha/20),
            min(255, bgB + alpha/20)
        ));
        setlinecolor(getfillcolor());
        fillcircle(CENTER_X, CENTER_Y, r);
    }
    
    // ========== 径向渐变填充表盘（使用主题配色）==========
    for (int r = R_MAIN; r > 0; r -= 2) {
        float t = (float)r / R_MAIN;
        
        // 从主题获取渐变色
        int centerR = GetRValue(currentTheme.dialCenterColor);
        int centerG = GetGValue(currentTheme.dialCenterColor);
        int centerB = GetBValue(currentTheme.dialCenterColor);
        int edgeR = GetRValue(currentTheme.dialEdgeColor);
        int edgeG = GetGValue(currentTheme.dialEdgeColor);
        int edgeB = GetBValue(currentTheme.dialEdgeColor);
        
        // 线性插值
        int r_color = (int)(edgeR + (centerR - edgeR) * t);
        int g_color = (int)(edgeG + (centerG - edgeG) * t);
        int b_color = (int)(edgeB + (centerB - edgeB) * t);
        
        setfillcolor(RGB(r_color, g_color, b_color));
        setlinecolor(RGB(r_color, g_color, b_color));
        solidcircle(CENTER_X, CENTER_Y, r);
    }
    
    // ========== 表盘边框（使用主题配色）==========
    setlinecolor(currentTheme.borderColor);
    setlinestyle(PS_SOLID, 3);
    circle(CENTER_X, CENTER_Y, R_MAIN);
    
    // 绘制小时刻度数字（使用主题文字颜色）
    for (int i = 0; i < 12; i++) {
        double angle = i * 3.14159265359 / 6 - 3.14159265359 / 2;
        
        settextcolor(currentTheme.textColor);
        setbkmode(TRANSPARENT);
        
        // 12, 3, 6, 9使用大字体，其他使用小字体
        if (i % 3 == 0) {
            settextstyle(32, 0, _T("Segoe UI"));
        } else {
            settextstyle(24, 0, _T("Segoe UI"));
        }
        
        int displayNum = (i == 0) ? 12 : i;
        wchar_t num[4];
        swprintf_s(num, L"%d", displayNum);
        
        // 计算文字位置（需要根据数字宽度调整）- 移到更外侧避免被指针遮挡
        int textWidth = (displayNum >= 10) ? 18 : 9;
        int textHeight = 14;
        int numX = CENTER_X + (int)(R_MAIN * 0.85 * cos(angle)) - textWidth;
        int numY = CENTER_Y + (int)(R_MAIN * 0.85 * sin(angle)) - textHeight;
        
        outtextxy(numX, numY, num);
    }
    
    // 绘制中心装饰圆（多层，立体感，使用主题配色）
    // 外圈边框
    COLORREF outerRing = RGB(
        (GetRValue(currentTheme.centerDotColor) + GetRValue(currentTheme.borderColor)) / 2,
        (GetGValue(currentTheme.centerDotColor) + GetGValue(currentTheme.borderColor)) / 2,
        (GetBValue(currentTheme.centerDotColor) + GetBValue(currentTheme.borderColor)) / 2
    );
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, 12, outerRing, false);
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, 10, currentTheme.centerDotColor, true);
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, 6, currentTheme.dialCenterColor, true);
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, 3, currentTheme.centerDotColor, true);
    
    // ========== 绘制模式切换按钮 ==========
    drawModeButton();
}

// 绘制数字时间显示
void drawDigitalTime(int hour, int minute, int second) {
    // 在屏幕下方显示数字时间
    const int DIGITAL_Y = WIN_H - 180;  // 距离底部的位置，给更多空间
    
    // 清除之前的数字时间区域（使用主题背景色）
    setfillcolor(currentTheme.bgColor);
    setlinecolor(currentTheme.bgColor);
    solidrectangle(0, DIGITAL_Y - 20, WIN_W, WIN_H);
    
    // ========== 绘制数字时间（使用主题文字颜色）==========
    settextcolor(currentTheme.textColor);
    setbkmode(TRANSPARENT);
    settextstyle(72, 0, _T("Consolas"));  // 增大字号到72
    
    // 格式化时间字符串 HH:MM:SS
    wchar_t timeStr[12];
    swprintf_s(timeStr, L"%02d:%02d:%02d", hour, minute, second);
    
    // 精确计算文字宽度并居中
    int timeWidth = textwidth(timeStr);
    int timeX = CENTER_X - timeWidth / 2;
    
    outtextxy(timeX, DIGITAL_Y, timeStr);
    
    // ========== 绘制日期信息（使用稍淡的文字颜色）==========
    settextstyle(28, 0, _T("Microsoft YaHei UI"));  // 使用微软雅黑，增大到28号
    // 使用主题文字颜色的80%透明度版本（通过RGB混合模拟）
    int textR = GetRValue(currentTheme.textColor);
    int textG = GetGValue(currentTheme.textColor);
    int textB = GetBValue(currentTheme.textColor);
    int bgR = GetRValue(currentTheme.bgColor);
    int bgG = GetGValue(currentTheme.bgColor);
    int bgB = GetBValue(currentTheme.bgColor);
    settextcolor(RGB(
        (textR * 6 + bgR * 4) / 10,  // 60%文字色 + 40%背景色
        (textG * 6 + bgG * 4) / 10,
        (textB * 6 + bgB * 4) / 10
    ));
    
    // 获取当前日期
    time_t now = time(NULL);
    struct tm t;
    localtime_s(&t, &now);
    
    wchar_t dateStr[50];
    const wchar_t* weekdays[] = {
        L"\u661F\u671F\u65E5",  // 星期日
        L"\u661F\u671F\u4E00",  // 星期一
        L"\u661F\u671F\u4E8C",  // 星期二
        L"\u661F\u671F\u4E09",  // 星期三
        L"\u661F\u671F\u56DB",  // 星期四
        L"\u661F\u671F\u4E94",  // 星期五
        L"\u661F\u671F\u516D"   // 星期六
    };
    swprintf_s(dateStr, L"%04d\u5E74%02d\u6708%02d\u65E5  %s", 
               t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, weekdays[t.tm_wday]);
    
    // 精确计算日期宽度并居中
    int dateWidth = textwidth(dateStr);
    int dateX = CENTER_X - dateWidth / 2;
    outtextxy(dateX, DIGITAL_Y + 90, dateStr);  // 调整垂直间距
}

// 全局变量：记录上一次的时间，避免不必要的重绘
static int lastHour = -1, lastMinute = -1, lastSecond = -1;
static bool needFullRedraw = true;

// 绘制时分秒针函数实现 - 防闪烁版本
void drawHands(int hour, int minute, int second) {
    // 检查时间是否改变
    bool timeChanged = (hour != lastHour || minute != lastMinute || second != lastSecond);
    
    // 只在时间改变时才重绘
    if (!timeChanged && !needFullRedraw) {
        return;
    }
    
    // 如果需要完全重绘表盘（初始化或窗口刷新）
    if (needFullRedraw) {
        init();
        needFullRedraw = false;
    } else {
        // 只清除指针区域，用径向渐变重绘以保持一致性
        const int R_MAIN = 310;
        const int CLEAR_RADIUS = 280;  // 清除半径，覆盖秒针全长(270+10)
        
        // 用径向渐变重绘指针区域（使用主题配色）
        for (int r = CLEAR_RADIUS; r > 0; r -= 2) {
            float t = (float)r / R_MAIN;
            
            int centerR = GetRValue(currentTheme.dialCenterColor);
            int centerG = GetGValue(currentTheme.dialCenterColor);
            int centerB = GetBValue(currentTheme.dialCenterColor);
            int edgeR = GetRValue(currentTheme.dialEdgeColor);
            int edgeG = GetGValue(currentTheme.dialEdgeColor);
            int edgeB = GetBValue(currentTheme.dialEdgeColor);
            
            int r_color = (int)(edgeR + (centerR - edgeR) * t);
            int g_color = (int)(edgeG + (centerG - edgeG) * t);
            int b_color = (int)(edgeB + (centerB - edgeB) * t);
            
            setfillcolor(RGB(r_color, g_color, b_color));
            setlinecolor(RGB(r_color, g_color, b_color));
            solidcircle(CENTER_X, CENTER_Y, r);
        }
        
        // 重绘被覆盖的数字（使用主题文字颜色）
        for (int i = 0; i < 12; i++) {
            double angle = i * 3.14159265359 / 6 - 3.14159265359 / 2;
            
            settextcolor(currentTheme.textColor);
            setbkmode(TRANSPARENT);
            
            if (i % 3 == 0) {
                settextstyle(32, 0, _T("Segoe UI"));
            } else {
                settextstyle(24, 0, _T("Segoe UI"));
            }
            
            int displayNum = (i == 0) ? 12 : i;
            wchar_t num[4];
            swprintf_s(num, L"%d", displayNum);
            
            int textWidth = (displayNum >= 10) ? 18 : 9;
            int textHeight = 14;
            int numX = CENTER_X + (int)(R_MAIN * 0.85 * cos(angle)) - textWidth;
            int numY = CENTER_Y + (int)(R_MAIN * 0.85 * sin(angle)) - textHeight;
            
            outtextxy(numX, numY, num);
        }
    }
    
    // 设置指针角度并渲染
    hourHand.setTimeAngle((hour % 12) * 5 + minute / 12, 60);
    minuteHand.setTimeAngle(minute, 60);
    secondHand.setTimeAngle(second, 60);
    
    // 按层次渲染所有指针（从粗到细，使用2xSSAA）
    hourHand.render();
    minuteHand.render();
    secondHand.render();
    
    // 绘制现代简洁的中心点（使用主题配色）
    AA_DrawCircle(CENTER_X, CENTER_Y, 8, currentTheme.centerDotColor, true);
    
    // 绘制数字时间显示
    drawDigitalTime(hour, minute, second);
    
    // 更新时间记录
    lastHour = hour;
    lastMinute = minute;
    lastSecond = second;
}

int main()
{
    // 完全隐藏控制台窗口
    HWND hwndConsole = GetConsoleWindow();
    if (hwndConsole != NULL) {
        ShowWindow(hwndConsole, SW_HIDE);
        // 释放控制台
        FreeConsole();
    }
    
    struct tm t;
    time_t now;
    time(&now);
    localtime_s(&t, &now);	// 获取当地时间
    
    // ========== 初始化主题（根据当前时间自动选择日夜模式）==========
    updateThemeAndHands(t.tm_hour);
    
    // 初始化图形窗口，隐藏控制台窗口
    initgraph(WIN_W, WIN_H);
    
    // 设置窗口标题
    SetWindowText(GetHWnd(), L"Modern Clock - Day/Night Mode");
    
    // 启用双缓冲，防止闪烁
    BeginBatchDraw();
    
    // 启用抗锯齿和平滑效果  
    setbkmode(TRANSPARENT);  // 设置透明背景模式，有助于抗锯齿
    
    init();			// 自定义图形初始化函数，用于绘制时钟界面
    drawDigitalTime(t.tm_hour, t.tm_min, t.tm_sec);  // 初始化时显示数字时间

    // 改用消息循环，支持窗口关闭
    ExMessage msg;
    int previousHour = t.tm_hour;  // 记录上一次的小时，用于检测主题切换时机
    
    while (true)
    {
        // 检查是否有窗口消息（处理所有待处理的消息）
        while (peekmessage(&msg, EX_MOUSE | EX_KEY | EX_WINDOW))
        {
            if (msg.message == WM_CLOSE)  // 点击关闭按钮
            {
                goto exit_loop;  // 退出外层循环
            }
            
            // ========== 检测鼠标点击按钮 ==========
            if (msg.message == WM_LBUTTONDOWN) {  // 鼠标左键按下
                if (isButtonClicked(msg.x, msg.y)) {
                    // 三态循环切换：自动模式 -> 日间模式 -> 夜间模式 -> 自动模式
                    if (isAutoMode) {
                        // 自动 -> 日间
                        isAutoMode = false;
                        isNightMode = false;
                        switchThemeManually(false);
                    } else if (!isNightMode) {
                        // 日间 -> 夜间
                        isNightMode = true;
                        switchThemeManually(true);
                    } else {
                        // 夜间 -> 自动
                        isAutoMode = true;
                        updateThemeAndHands(t.tm_hour);
                    }
                    
                    // 重新绘制整个界面
                    needFullRedraw = true;
                    init();
                    drawDigitalTime(t.tm_hour, t.tm_min, t.tm_sec);
                    FlushBatchDraw();
                }
            }
        }
        
        /* 不停获取当前时间，与之前绘制时/分/秒针时的时间相比，
        以判断新时间的到来 */
        time(&now);
        localtime_s(&t, &now);
        
        // ========== 检测小时变化，自动模式下切换日夜主题 ==========
        if (isAutoMode && t.tm_hour != previousHour) {
            // 检查是否需要切换主题（6点和18点）
            if ((previousHour == 5 && t.tm_hour == 6) ||      // 早上6点切换到日间
                (previousHour == 17 && t.tm_hour == 18)) {    // 晚上18点切换到夜间
                updateThemeAndHands(t.tm_hour);
                needFullRedraw = true;  // 标记需要完全重绘
            }
            previousHour = t.tm_hour;
        }
        
        /* 根据新的时间重新绘制时针，分针，秒针，
        可以自定义函数完成，注意坐标数据类型 */
        drawHands(t.tm_hour, t.tm_min, t.tm_sec);
        
        // 刷新显示缓冲区，防止闪烁
        FlushBatchDraw();
        
        Sleep(100);  // 避免过度占用CPU
    }
    
exit_loop:  // 退出标签
    EndBatchDraw();     // 结束双缓冲
    closegraph();		// 退出图形界面
    return 0;
}