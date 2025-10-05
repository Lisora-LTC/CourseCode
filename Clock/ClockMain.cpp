
#include <graphics.h>   // EasyX 图形库
#include <conio.h>      // _kbhit(), _getch()
#include <ctime>        // time(), localtime_s()
#include <cmath>        // sin(), cos() 用于计算指针坐标
#include <Windows.h>    // Sleep() 函数
#include <algorithm>    // min, max 函数

// 全局中心点和分辨率参数
#define WIN_W 1280
#define WIN_H 960
const int CENTER_X = WIN_W / 2;
const int CENTER_Y = WIN_H / 2;

// 抗锯齿工具函数
namespace AntiAlias {
    // 混合两个颜色，alpha范围0-1
    COLORREF blendColor(COLORREF bg, COLORREF fg, float alpha) {
        if (alpha <= 0) return bg;
        if (alpha >= 1) return fg;
        
        int bgR = GetRValue(bg), bgG = GetGValue(bg), bgB = GetBValue(bg);
        int fgR = GetRValue(fg), fgG = GetGValue(fg), fgB = GetBValue(fg);
        
        int r = (int)(bgR * (1 - alpha) + fgR * alpha);
        int g = (int)(bgG * (1 - alpha) + fgG * alpha);
        int b = (int)(bgB * (1 - alpha) + fgB * alpha);
        
        return RGB(r, g, b);
    }
    
    // 获取指定位置的像素颜色（安全版本）
    COLORREF getPixelSafe(int x, int y) {
        if (x < 0 || y < 0 || x >= WIN_W || y >= WIN_H) {
            return RGB(239, 246, 255); // 返回背景色
        }
        return getpixel(x, y);
    }
    
    // 设置带透明度的像素
    void setPixelAlpha(int x, int y, COLORREF color, float alpha) {
        if (x < 0 || y < 0 || x >= WIN_W || y >= WIN_H) return;
        
        COLORREF bgColor = getPixelSafe(x, y);
        COLORREF blended = blendColor(bgColor, color, alpha);
        putpixel(x, y, blended);
    }
    
    // Wu's 抗锯齿直线算法
    void drawAntiAliasedLine(int x1, int y1, int x2, int y2, COLORREF color, int thickness = 1) {
        // 处理粗线条：绘制多条平行线
        if (thickness > 1) {
            float dx = y2 - y1;
            float dy = x1 - x2;
            float len = sqrt(dx * dx + dy * dy);
            if (len > 0) {
                dx /= len;
                dy /= len;
            }
            
            for (int t = -(thickness / 2); t <= thickness / 2; t++) {
                int offset_x1 = x1 + (int)(t * dx);
                int offset_y1 = y1 + (int)(t * dy);
                int offset_x2 = x2 + (int)(t * dx);
                int offset_y2 = y2 + (int)(t * dy);
                drawAntiAliasedLine(offset_x1, offset_y1, offset_x2, offset_y2, color, 1);
            }
            return;
        }
        
        // Wu's 算法核心
        bool steep = abs(y2 - y1) > abs(x2 - x1);
        
        if (steep) {
            std::swap(x1, y1);
            std::swap(x2, y2);
        }
        if (x1 > x2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        
        float dx = x2 - x1;
        float dy = y2 - y1;
        float gradient = (dx == 0) ? 1.0f : dy / dx;
        
        // 处理端点
        float xend = x1;
        float yend = y1 + gradient * (xend - x1);
        float xgap = 1 - (x1 + 0.5f - floor(x1 + 0.5f));
        int xpxl1 = (int)xend;
        int ypxl1 = (int)floor(yend);
        
        if (steep) {
            setPixelAlpha(ypxl1, xpxl1, color, (1 - (yend - floor(yend))) * xgap);
            setPixelAlpha(ypxl1 + 1, xpxl1, color, (yend - floor(yend)) * xgap);
        } else {
            setPixelAlpha(xpxl1, ypxl1, color, (1 - (yend - floor(yend))) * xgap);
            setPixelAlpha(xpxl1, ypxl1 + 1, color, (yend - floor(yend)) * xgap);
        }
        
        float intery = yend + gradient;
        
        xend = x2;
        yend = y2 + gradient * (xend - x2);
        xgap = x2 + 0.5f - floor(x2 + 0.5f);
        int xpxl2 = (int)xend;
        int ypxl2 = (int)floor(yend);
        
        if (steep) {
            setPixelAlpha(ypxl2, xpxl2, color, (1 - (yend - floor(yend))) * xgap);
            setPixelAlpha(ypxl2 + 1, xpxl2, color, (yend - floor(yend)) * xgap);
        } else {
            setPixelAlpha(xpxl2, ypxl2, color, (1 - (yend - floor(yend))) * xgap);
            setPixelAlpha(xpxl2, ypxl2 + 1, color, (yend - floor(yend)) * xgap);
        }
        
        // 主循环
        for (int x = xpxl1 + 1; x < xpxl2; x++) {
            if (steep) {
                setPixelAlpha((int)floor(intery), x, color, 1 - (intery - floor(intery)));
                setPixelAlpha((int)floor(intery) + 1, x, color, intery - floor(intery));
            } else {
                setPixelAlpha(x, (int)floor(intery), color, 1 - (intery - floor(intery)));
                setPixelAlpha(x, (int)floor(intery) + 1, color, intery - floor(intery));
            }
            intery += gradient;
        }
    }
    
    // 抗锯齿圆圈绘制
    void drawAntiAliasedCircle(int centerX, int centerY, int radius, COLORREF color, bool filled = true) {
        for (int y = -radius - 1; y <= radius + 1; y++) {
            for (int x = -radius - 1; x <= radius + 1; x++) {
                float distance = sqrt(x * x + y * y);
                float alpha = 0;
                
                if (filled) {
                    // 实心圆
                    if (distance <= radius - 0.5f) {
                        alpha = 1.0f;
                    } else if (distance < radius + 0.5f) {
                        alpha = radius + 0.5f - distance;
                    }
                } else {
                    // 空心圆（圆环）
                    float innerRadius = radius - 0.5f;
                    float outerRadius = radius + 0.5f;
                    if (distance >= innerRadius && distance <= outerRadius) {
                        if (distance <= radius) {
                            alpha = distance - innerRadius;
                        } else {
                            alpha = outerRadius - distance;
                        }
                    }
                }
                
                if (alpha > 0) {
                    setPixelAlpha(centerX + x, centerY + y, color, alpha);
                }
            }
        }
    }
}

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
            AntiAlias::drawAntiAliasedLine(centerX, centerY, endX, endY, 
                                         RGB(255, 255, 255), thickness + 2);
        }

        // 绘制主指针线条（使用抗锯齿）
        AntiAlias::drawAntiAliasedLine(centerX, centerY, endX, endY, color, thickness);
        
        // 为指针添加抗锯齿圆润端点
        if (thickness > 1) {
            AntiAlias::drawAntiAliasedCircle(endX, endY, 1, color, true);
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

// 现代浅蓝配色方案 - 增强指针对比度
#define DARK_BLUE    RGB(15, 35, 80)     // 更深的蓝色 - 时针，增强对比
#define OCEAN_BLUE   RGB(30, 80, 180)    // 更深的海蓝 - 分针，增强对比  
#define BRIGHT_BLUE  RGB(220, 38, 127)   // 改为粉红色 - 秒针，强烈对比
#define SOFT_BLUE    RGB(239, 246, 255)  // 柔和蓝 - 背景色
#define PURE_WHITE   RGB(255, 255, 255)  // 纯白色 - 高亮元素
#define GRAY_BLUE    RGB(148, 163, 184)  // 灰蓝色 - 次要刻度

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
    for (int r = 400; r > 0; r -= 40) {
        int alpha = 255 - (r * 50 / 400);  // 透明度渐变
        setfillcolor(RGB(239 + alpha/20, 246 + alpha/30, 255));
        setlinecolor(RGB(239 + alpha/20, 246 + alpha/30, 255));
        fillcircle(CENTER_X, CENTER_Y, r);
    }
    
    // 绘制主表盘 - 先用传统方法绘制填充
    setfillcolor(PURE_WHITE);
    fillcircle(CENTER_X, CENTER_Y, R_MAIN);
    
    // 使用抗锯齿绘制边框
    AntiAlias::drawAntiAliasedCircle(CENTER_X, CENTER_Y, R_MAIN, RGB(148, 163, 184), false);
    AntiAlias::drawAntiAliasedCircle(CENTER_X, CENTER_Y, R_MAIN - 10, RGB(226, 232, 240), false);
    
    // 添加高光效果（左上角）
    setlinecolor(RGB(255, 255, 255));
    setlinestyle(PS_SOLID, 1);
    for (int i = 0; i < 30; i++) {
        double lightAngle = -2.356 + i * 0.05;  // 从左上开始的弧线
    int x1 = CENTER_X + (int)(R_MAIN * 0.47 * cos(lightAngle));
    int y1 = CENTER_Y + (int)(R_MAIN * 0.47 * sin(lightAngle));
    int x2 = CENTER_X + (int)(R_MAIN * 0.48 * cos(lightAngle));
    int y2 = CENTER_Y + (int)(R_MAIN * 0.48 * sin(lightAngle));
        line(x1, y1, x2, y2);
    }
    
    // 绘制小时刻度 - 现代极简风格
    for (int i = 0; i < 12; i++) {
        double angle = i * 3.14159265359 / 6 - 3.14159265359 / 2;
        
        if (i % 3 == 0) {
            // 主要刻度 (12, 3, 6, 9点位置) - 优雅的小圆点
            int dotX = CENTER_X + (int)(R_MAIN * 0.85 * cos(angle));
            int dotY = CENTER_Y + (int)(R_MAIN * 0.85 * sin(angle));
            AntiAlias::drawAntiAliasedCircle(dotX, dotY, 6, DARK_BLUE, true);
            
            // 绘制数字 - 精致字体
            settextcolor(DARK_BLUE);
            setbkmode(TRANSPARENT);
            settextstyle(28, 0, _T("Segoe UI"));
            int numX = CENTER_X + (int)(R_MAIN * 0.70 * cos(angle)) - 14;
            int numY = CENTER_Y + (int)(R_MAIN * 0.70 * sin(angle)) - 14;
            
            wchar_t num[3];
            int displayNum;
            if (i == 0) displayNum = 12;        // 0*30° = 12点
            else if (i == 3) displayNum = 3;    // 3*30° = 3点  
            else if (i == 6) displayNum = 6;    // 6*30° = 6点
            else if (i == 9) displayNum = 9;    // 9*30° = 9点
            else displayNum = i;                // 其他位置（实际不会执行到这里）
            
            swprintf_s(num, L"%d", displayNum);
            outtextxy(numX, numY, num);
        } else {
            // 次要刻度 - 精致的小点
            int dotX = CENTER_X + (int)(R_MAIN * 0.88 * cos(angle));
            int dotY = CENTER_Y + (int)(R_MAIN * 0.88 * sin(angle));
            AntiAlias::drawAntiAliasedCircle(dotX, dotY, 2, GRAY_BLUE, true);
        }
    }
    
    // 绘制中心点 - 抗锯齿版本
    AntiAlias::drawAntiAliasedCircle(CENTER_X, CENTER_Y, 8, DARK_BLUE, true);
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
        
        // 重绘被覆盖的刻度点
        const int R_MAIN = 310;
        for (int i = 0; i < 12; i++) {
            double angle = i * 3.14159265359 / 6 - 3.14159265359 / 2;
            
            if (i % 3 == 0) {
                int dotX = CENTER_X + (int)(R_MAIN * 0.85 * cos(angle));
                int dotY = CENTER_Y + (int)(R_MAIN * 0.85 * sin(angle));
                AntiAlias::drawAntiAliasedCircle(dotX, dotY, 6, DARK_BLUE, true);
                
                // 重绘数字
                settextcolor(DARK_BLUE);
                setbkmode(TRANSPARENT);
                settextstyle(28, 0, _T("Segoe UI"));
                int numX = CENTER_X + (int)(R_MAIN * 0.70 * cos(angle)) - 14;
                int numY = CENTER_Y + (int)(R_MAIN * 0.70 * sin(angle)) - 14;
                
                wchar_t num[3];
                int displayNum;
                if (i == 0) displayNum = 12;
                else if (i == 3) displayNum = 3;
                else if (i == 6) displayNum = 6;
                else if (i == 9) displayNum = 9;
                else displayNum = i;
                
                swprintf_s(num, L"%d", displayNum);
                outtextxy(numX, numY, num);
            } else {
                int dotX = CENTER_X + (int)(R_MAIN * 0.88 * cos(angle));
                int dotY = CENTER_Y + (int)(R_MAIN * 0.88 * sin(angle));
                AntiAlias::drawAntiAliasedCircle(dotX, dotY, 2, GRAY_BLUE, true);
            }
        }
    }
    
    // 设置指针角度并渲染
    hourHand.setTimeAngle((hour % 12) * 5 + minute / 12, 60);
    minuteHand.setTimeAngle(minute, 60);
    secondHand.setTimeAngle(second, 60);
    
    // 按层次渲染所有指针（从粗到细）
    hourHand.render();
    minuteHand.render();
    secondHand.render();
    
    // 绘制现代简洁的中心点（抗锯齿）
    AntiAlias::drawAntiAliasedCircle(CENTER_X, CENTER_Y, 8, DARK_BLUE, true);
    
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