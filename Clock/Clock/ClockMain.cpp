
#include <graphics.h>   // EasyX 图形库
#include <conio.h>      // _kbhit(), _getch()
#include <ctime>        // time(), localtime_s()
#include <cmath>        // sin(), cos() 用于计算指针坐标
#include <Windows.h>    // Sleep() 函数
#include <algorithm>    // min, max 函数
#include "AntiAlias.h"  // 抗锯齿算法头文件

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

// ========== 现代配色方案 - 简约风格 ==========
// 当前：浅蓝主题
#define DARK_BLUE    RGB(15, 35, 80)     // 深蓝色 - 时针
#define OCEAN_BLUE   RGB(30, 80, 180)    // 海蓝色 - 分针
#define BRIGHT_BLUE  RGB(220, 38, 127)   // 粉红色 - 秒针（亮色对比）
#define SOFT_BLUE    RGB(239, 246, 255)  // 柔和蓝 - 背景色
#define PURE_WHITE   RGB(255, 255, 255)  // 纯白色 - 表盘
#define GRAY_BLUE    RGB(148, 163, 184)  // 灰蓝色 - 次要刻度
#define ACCENT_GOLD  RGB(255, 193, 7)    // 金色点缀（可选）

/* ========== 其他配色方案建议 ==========
 * 
 * 🎨 深色主题（高级感）：
 * - 背景：RGB(18, 18, 18) 或 RGB(25, 28, 35)
 * - 表盘：RGB(40, 44, 52)
 * - 指针：RGB(255, 255, 255) / RGB(100, 200, 255) / RGB(255, 70, 70)
 * 
 * 🌿 绿色主题（清新）：
 * - 背景：RGB(240, 248, 245)
 * - 表盘：RGB(255, 255, 255)
 * - 指针：RGB(46, 125, 50) / RGB(56, 142, 60) / RGB(255, 152, 0)
 * 
 * 🌸 粉色主题（柔和）：
 * - 背景：RGB(255, 245, 250)
 * - 表盘：RGB(255, 255, 255)
 * - 指针：RGB(233, 30, 99) / RGB(236, 64, 122) / RGB(240, 98, 146)
 * 
 * 🌊 渐变主题（动感）：
 * - 背景：从 RGB(106, 17, 203) 到 RGB(37, 117, 252)
 * - 表盘：半透明 RGBA
 * - 指针：RGB(255, 255, 255) 纯白
 * 
 * 💎 现代元素建议：
 * - 微光效果：边框处添加细微高光
 * - 阴影：表盘下方添加柔和阴影
 * - 动画：秒针平滑过渡（需要更高刷新率）
 * - 材质：玻璃拟态效果（毛玻璃背景）
 * - 装饰：四个基点添加小图标或点缀
 */

// 全局指针对象 - 增强对比度版本
ClockHand hourHand(CENTER_X, CENTER_Y, 150, 0, DARK_BLUE, 10);     // 时针
ClockHand minuteHand(CENTER_X, CENTER_Y, 230, 0, OCEAN_BLUE, 6);  // 分针
ClockHand secondHand(CENTER_X, CENTER_Y, 270, 0, BRIGHT_BLUE, 4); // 秒针

// 前置声明，具体实现可以后续添加
void init();            // 初始化时钟界面
void drawClock();       // 绘制时钟表盘
void drawHands(int hour, int minute, int second);  // 绘制时分秒针

// 初始化函数实现 - 现代浅蓝风格
void init() {
    const int R_MAIN = 310;
    setbkcolor(SOFT_BLUE);      // 设置背景色为柔和浅蓝色
    cleardevice();              // 清屏
    
    // 绘制背景渐变效果（用多个同心圆模拟）
    for (int r = 360; r > 0; r -= 40) {
        int alpha = 255 - (r * 50 / 360);  // 透明度渐变
        setfillcolor(RGB(239 + alpha/20, 246 + alpha/30, 255));
        setlinecolor(RGB(239 + alpha/20, 246 + alpha/30, 255));
        fillcircle(CENTER_X, CENTER_Y, r);
    }
    
    // 绘制主表盘 - 先用传统方法绘制填充
    setfillcolor(PURE_WHITE);
    fillcircle(CENTER_X, CENTER_Y, R_MAIN);
    
    // ========== 简洁深蓝色边框 ==========
    // 使用传统方法绘制边框，避免抗锯齿干涉
    setlinecolor(RGB(70, 100, 140));  // 深蓝色
    setlinestyle(PS_SOLID, 3);  // 3像素宽的深蓝色边框
    circle(CENTER_X, CENTER_Y, R_MAIN);
    
    // 绘制小时刻度 - 带数字的完整版本（不再绘制小圆点）
    for (int i = 0; i < 12; i++) {
        double angle = i * 3.14159265359 / 6 - 3.14159265359 / 2;
        
        // 绘制所有12个数字
        settextcolor(DARK_BLUE);
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
    
    // 绘制中心装饰圆（多层，立体感）
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, 12, RGB(100, 120, 150), false);
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, 10, DARK_BLUE, true);
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, 6, RGB(200, 210, 220), true);
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, 3, DARK_BLUE, true);
}

// 绘制数字时间显示
void drawDigitalTime(int hour, int minute, int second) {
    // 在屏幕下方显示数字时间
    const int DIGITAL_Y = WIN_H - 180;  // 距离底部的位置，给更多空间
    
    // 清除之前的数字时间区域（扩大清除范围）
    setfillcolor(SOFT_BLUE);
    setlinecolor(SOFT_BLUE);
    solidrectangle(0, DIGITAL_Y - 20, WIN_W, WIN_H);
    
    // ========== 绘制数字时间 ==========
    settextcolor(DARK_BLUE);
    setbkmode(TRANSPARENT);
    settextstyle(72, 0, _T("Consolas"));  // 增大字号到72
    
    // 格式化时间字符串 HH:MM:SS
    wchar_t timeStr[12];
    swprintf_s(timeStr, L"%02d:%02d:%02d", hour, minute, second);
    
    // 精确计算文字宽度并居中
    int timeWidth = textwidth(timeStr);
    int timeX = CENTER_X - timeWidth / 2;
    
    outtextxy(timeX, DIGITAL_Y, timeStr);
    
    // ========== 绘制日期信息 ==========
    settextstyle(28, 0, _T("Microsoft YaHei UI"));  // 使用微软雅黑，增大到28号
    settextcolor(GRAY_BLUE);
    
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
        // 只清除指针区域，不重绘整个表盘
        // 用背景色覆盖指针区域
        setfillcolor(PURE_WHITE);
        setlinecolor(PURE_WHITE);
        solidcircle(CENTER_X, CENTER_Y, 280);  // 清除指针区域，确保覆盖秒针全长(270+10)
        
        // 重绘被覆盖的数字
        const int R_MAIN = 310;
        for (int i = 0; i < 12; i++) {
            double angle = i * 3.14159265359 / 6 - 3.14159265359 / 2;
            
            // 重绘数字（不再绘制小圆点）
            settextcolor(DARK_BLUE);
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
    
    // 绘制现代简洁的中心点（动态，使用2xSSAA）
    AA_DrawCircle(CENTER_X, CENTER_Y, 8, DARK_BLUE, true);
    
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
    
    // 初始化图形窗口，隐藏控制台窗口
    initgraph(WIN_W, WIN_H);
    
    // 设置窗口标题
    SetWindowText(GetHWnd(), L"Modern Clock");
    
    // 启用双缓冲，防止闪烁
    BeginBatchDraw();
    
    // 启用抗锯齿和平滑效果  
    setbkmode(TRANSPARENT);  // 设置透明背景模式，有助于抗锯齿
    
    init();			// 自定义图形初始化函数，用于绘制时钟界面
    drawDigitalTime(t.tm_hour, t.tm_min, t.tm_sec);  // 初始化时显示数字时间

    // 改用消息循环，支持窗口关闭
    ExMessage msg;
    while (true)
    {
        // 检查是否有窗口消息
        if (peekmessage(&msg, EX_MOUSE | EX_KEY | EX_WINDOW))
        {
            if (msg.message == WM_CLOSE)  // 点击关闭按钮
            {
                break;
            }
        }
        
        /* 不停获取当前时间，与之前绘制时/分/秒针时的时间相比，
        以判断新时间的到来 */
        time(&now);
        localtime_s(&t, &now);
        
        /* 根据新的时间重新绘制时针，分针，秒针，
        可以自定义函数完成，注意坐标数据类型 */
        drawHands(t.tm_hour, t.tm_min, t.tm_sec);
        
        // 刷新显示缓冲区，防止闪烁
        FlushBatchDraw();
        
        Sleep(100);  // 避免过度占用CPU
    }
    
    EndBatchDraw();     // 结束双缓冲
    closegraph();		// 退出图形界面
    return 0;
}