
#include <graphics.h>   // EasyX
#include <conio.h>
#include <ctime>
#include <cmath>
#include <Windows.h>
#include <algorithm>
#include "AntiAlias.h"
#include "ThemeManager.h"

// 窗口与中心点
#define WIN_W 1280
#define WIN_H 960
const int CENTER_X = WIN_W / 2;
const int CENTER_Y = 400;  // 向上移动，给下方文字留出空间（原来是 WIN_H / 2 = 480）

// 抗锯齿：1=2xSSAA，0=Wu's
#define USE_2X_SSAA 1
#define USE_2X_SSAA_FOR_STATIC 1  

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

// 静态元素（刻度/边框/中心点）
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
    int centerX;
    int centerY;
    double length;
    double angle;   // 弧度，0 指向12点
    COLORREF color;
    int thickness;

public:
    ClockHand(int cx = CENTER_X, int cy = CENTER_Y, double len = 100, 
              double ang = 0, COLORREF col = WHITE, int thick = 2)
        : centerX(cx), centerY(cy), length(len), angle(ang), 
          color(col), thickness(thick) {}

    void setCenter(int x, int y) { centerX = x; centerY = y; }
    void setLength(double len) { length = len; }
    void setAngle(double ang) { angle = ang; }
    void setColor(COLORREF col) { color = col; }
    void setThickness(int thick) { thickness = thick; }

    double getAngle() const { return angle; }
    double getLength() const { return length; }
    COLORREF getColor() const { return color; }

    void setTimeAngle(int value, int maxValue) {
        angle = (value * 2.0 * 3.14159265359 / maxValue) - 3.14159265359 / 2.0;
    }

    void render() const {
        int endX = centerX + (int)(length * cos(angle));
        int endY = centerY + (int)(length * sin(angle));

        // 白色描边（粗针增强对比）
        if (thickness >= 3) {
            AA_DrawLine(centerX, centerY, endX, endY, RGB(255, 255, 255), thickness + 2);
        }

        // 主线
        AA_DrawLine(centerX, centerY, endX, endY, color, thickness);

        // 圆润端点
        if (thickness > 1) {
            AA_DrawCircle(endX, endY, 1, color, true);
        }
    }

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
void applyVignette();   // 应用暗角效果
void drawShadowCircle(int cx, int cy, int r, COLORREF color);  // 绘制柔和投影圆


// 初始化函数实现 - 使用当前主题配色
void init() {
    const int R_MAIN = 310;
    setbkcolor(currentTheme.bgColor);  // 使用主题背景色
    cleardevice();                     // 清屏
    
    // 暗角（底层）
    applyVignette();
    
    // 背景微光圈层
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
    
    // 表盘软投影
    drawShadowCircle(CENTER_X, CENTER_Y, R_MAIN, RGB(0, 0, 0));
    
    // 表盘径向渐变
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
    
    // 表盘边框（静态AA）
    // 使用两条相邻的AA圆形成近似3px的边框效果
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, R_MAIN, currentTheme.borderColor, false);
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, R_MAIN - 1, currentTheme.borderColor, false);
    
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
    
    // 绘制中心装饰圆（使用与drawHands一致的样式）
    // 先绘制同心阴影层（从大到小，逐渐变暗）
    for (int i = 3; i > 0; i--) {
        float alphaRatio = 0.1f * i / 3.0f;
        int bgR = GetRValue(currentTheme.bgColor);
        int bgG = GetGValue(currentTheme.bgColor);
        int bgB = GetBValue(currentTheme.bgColor);
        int shadowR = isNightMode ? 0 : 20;
        int shadowG = isNightMode ? 0 : 20;
        int shadowB = isNightMode ? 0 : 30;
        int mixR = (int)(bgR * (1 - alphaRatio) + shadowR * alphaRatio);
        int mixG = (int)(bgG * (1 - alphaRatio) + shadowG * alphaRatio);
        int mixB = (int)(bgB * (1 - alphaRatio) + shadowB * alphaRatio);
        setfillcolor(RGB(mixR, mixG, mixB));
        setlinecolor(RGB(mixR, mixG, mixB));
        solidcircle(CENTER_X, CENTER_Y, 8 + i);
    }
    // 绘制中心点主体
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, 8, currentTheme.centerDotColor, true);
    
    // 模式切换按钮
    drawModeButton();
}

// 暗角（Vignette）
void applyVignette() {
    // 暗角参数（在表盘之前绘制，可以更宽更强）
    const int vignetteWidth = 140;  // 暗角宽度（像素）- 增大到140
    const int maxAlpha = isNightMode ? 70 : 50;  // 最大不透明度 - 增强效果
    
    int bgR = GetRValue(currentTheme.bgColor);
    int bgG = GetGValue(currentTheme.bgColor);
    int bgB = GetBValue(currentTheme.bgColor);
    
    // 从外向内绘制渐变（边缘最暗，向内渐亮）
    // 逐像素精细渐变，与角落保持一致
    for (int dist = 0; dist < vignetteWidth; dist++) {
        // dist=0 时在最外边（最暗），dist=vignetteWidth-1 时在最内侧（最亮）
        float t = (float)dist / vignetteWidth;  // 0到1，从外到内
        float alpha = maxAlpha * (1.0f - t) * (1.0f - t) / 255.0f;  // 外侧暗，内侧亮
        
        // 混合黑色和背景色（alpha 越大越暗）
        int r_mix = (int)(bgR * (1.0f - alpha));
        int g_mix = (int)(bgG * (1.0f - alpha));
        int b_mix = (int)(bgB * (1.0f - alpha));
        
        COLORREF vignetteColor = RGB(r_mix, g_mix, b_mix);
        setfillcolor(vignetteColor);
        setlinecolor(vignetteColor);
        
        // 绘制四条边的渐变（不包括角落，逐像素绘制1像素宽的线）
        // 上边（排除左右角）
        line(vignetteWidth, dist, WIN_W - vignetteWidth - 1, dist);
        // 下边（排除左右角）
        line(vignetteWidth, WIN_H - dist - 1, WIN_W - vignetteWidth - 1, WIN_H - dist - 1);
        // 左边（排除上下角）
        line(dist, vignetteWidth, dist, WIN_H - vignetteWidth - 1);
        // 右边（排除上下角）
        line(WIN_W - dist - 1, vignetteWidth, WIN_W - dist - 1, WIN_H - vignetteWidth - 1);
    }
    
    // 角落渐变（与四边衔接）
    // 角落需要填充整个正方形区域，使用到内侧的最短距离确保与四边衔接
    for (int x = 0; x < vignetteWidth; x++) {
        for (int y = 0; y < vignetteWidth; y++) {
            // 使用到内侧的最短距离（min(x,y)），确保在边界处与四边渐变一致
            float dist = (float)min(x, y);
            float t = dist / vignetteWidth;  // 归一化到 0-1
            
            // 计算透明度（与四边完全一致）
            float alpha = maxAlpha * (1.0f - t) * (1.0f - t) / 255.0f;
            
            int r_mix = (int)(bgR * (1.0f - alpha));
            int g_mix = (int)(bgG * (1.0f - alpha));
            int b_mix = (int)(bgB * (1.0f - alpha));
            
            COLORREF vignetteColor = RGB(r_mix, g_mix, b_mix);
            
            setfillcolor(vignetteColor);
            setlinecolor(vignetteColor);
            // 左上角
            putpixel(x, y, vignetteColor);
            // 右上角
            putpixel(WIN_W - x - 1, y, vignetteColor);
            // 左下角
            putpixel(x, WIN_H - y - 1, vignetteColor);
            // 右下角
            putpixel(WIN_W - x - 1, WIN_H - y - 1, vignetteColor);
        }
    }
}

// 软投影绘制
void drawShadowCircle(int cx, int cy, int r, COLORREF color) {
    // 绘制多层模糊投影（简单的软投影实现）
    const int shadowLayers = 5;      // 投影层数
    const int shadowOffsetX = 3;     // 投影X偏移
    const int shadowOffsetY = 3;     // 投影Y偏移
    
    // 从最外层开始绘制，逐渐变亮
    for (int i = shadowLayers; i > 0; i--) {
        // 计算透明度（通过与背景色混合模拟）
        float alphaRatio = 0.15f * i / shadowLayers;  // 每层透明度递减
        
        // 提取背景色和投影色
        int bgR = GetRValue(currentTheme.bgColor);
        int bgG = GetGValue(currentTheme.bgColor);
        int bgB = GetBValue(currentTheme.bgColor);
        
        // 投影色为深灰色/黑色
        int shadowR = isNightMode ? 0 : 20;
        int shadowG = isNightMode ? 0 : 20;
        int shadowB = isNightMode ? 0 : 30;
        
        // 混合背景色和投影色
        int mixR = (int)(bgR * (1 - alphaRatio) + shadowR * alphaRatio);
        int mixG = (int)(bgG * (1 - alphaRatio) + shadowG * alphaRatio);
        int mixB = (int)(bgB * (1 - alphaRatio) + shadowB * alphaRatio);
        
        COLORREF shadowColor = RGB(mixR, mixG, mixB);
        
        // 绘制投影圆（每层稍大一点，偏移量稍大一点）
        int offsetScale = i;
        setfillcolor(shadowColor);
        setlinecolor(shadowColor);
        solidcircle(cx + shadowOffsetX * offsetScale / shadowLayers,
                   cy + shadowOffsetY * offsetScale / shadowLayers,
                   r + i - 1);
    }
}

// 绘制数字时间显示
void drawDigitalTime(int hour, int minute, int second) {
    // 底部数字时间与日期
    const int DIGITAL_Y = WIN_H - 180;  // 距离底部的位置，给更多空间
    
    // 清底部区域
    setfillcolor(currentTheme.bgColor);
    setlinecolor(currentTheme.bgColor);
    solidrectangle(0, DIGITAL_Y - 20, WIN_W, WIN_H);
    
    // 重新绘制底部暗角（因为被清除了）- 与主暗角函数保持一致
    const int vignetteWidth = 140;
    const int maxAlpha = isNightMode ? 70 : 50;
    int bg_r = GetRValue(currentTheme.bgColor);
    int bg_g = GetGValue(currentTheme.bgColor);
    int bg_b = GetBValue(currentTheme.bgColor);
    
    // 逐像素精细渐变
    for (int dist = 0; dist < vignetteWidth; dist++) {
        float t = (float)dist / vignetteWidth;
        float alpha = maxAlpha * (1.0f - t) * (1.0f - t) / 255.0f;
        
        int r_mix = (int)(bg_r * (1.0f - alpha));
        int g_mix = (int)(bg_g * (1.0f - alpha));
        int b_mix = (int)(bg_b * (1.0f - alpha));
        
        COLORREF vignetteColor = RGB(r_mix, g_mix, b_mix);
        setlinecolor(vignetteColor);
        
        // 只重绘底部区域的暗角（如果在清除范围内）
        int bottomY = WIN_H - dist - 1;
        if (bottomY >= DIGITAL_Y - 20) {
            // 底边（排除左右角）
            line(vignetteWidth, bottomY, WIN_W - vignetteWidth - 1, bottomY);
        }
        
        // 左边和右边（从数字区域顶部到底部）
        if (dist < vignetteWidth) {
            int leftTopY = max(DIGITAL_Y - 20, vignetteWidth);
            int leftBottomY = WIN_H - vignetteWidth - 1;
            if (leftBottomY >= DIGITAL_Y - 20) {
                // 左边
                line(dist, leftTopY, dist, leftBottomY);
                // 右边
                line(WIN_W - dist - 1, leftTopY, WIN_W - dist - 1, leftBottomY);
            }
        }
    }
    
    // 底部角落复原
    for (int x = 0; x < vignetteWidth; x++) {
        for (int y = 0; y < vignetteWidth; y++) {
            // 使用到内侧的最短距离（min(x,y)），与四边衔接
            float dist = (float)min(x, y);
            float t = dist / vignetteWidth;
            
            float alpha = maxAlpha * (1.0f - t) * (1.0f - t) / 255.0f;
            
            int r_mix = (int)(bg_r * (1.0f - alpha));
            int g_mix = (int)(bg_g * (1.0f - alpha));
            int b_mix = (int)(bg_b * (1.0f - alpha));
            
            COLORREF vignetteColor = RGB(r_mix, g_mix, b_mix);
            
            // 只重绘被清除的底部区域
            int bottomStartY = DIGITAL_Y - 20;
            
            // 左下角（只绘制 Y >= bottomStartY 的部分）
            int leftY = WIN_H - y - 1;
            if (leftY >= bottomStartY) {
                putpixel(x, leftY, vignetteColor);
            }
            
            // 右下角（只绘制 Y >= bottomStartY 的部分）
            int rightY = WIN_H - y - 1;
            if (rightY >= bottomStartY) {
                putpixel(WIN_W - x - 1, rightY, vignetteColor);
            }
        }
    }
    
    // 数字时间
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
    
    // 日期信息
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
    // 时间变更判定
    bool timeChanged = (hour != lastHour || minute != lastMinute || second != lastSecond);
    
    // 未变更且无需全量 → 早退
    if (!timeChanged && !needFullRedraw) {
        return;
    }
    
    // 全量重绘
    if (needFullRedraw) {
        init();
        needFullRedraw = false;
    } else {
    // 局部回填：指针覆盖区域（径向渐变）
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
        
    // 复原刻度数字
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
    
    // 渲染指针
    hourHand.render();
    minuteHand.render();
    secondHand.render();
    
    // 中心点：同心阴影 + 实心主体
    for (int i = 3; i > 0; i--) {
        float alphaRatio = 0.1f * i / 3.0f;
        int bgR = GetRValue(currentTheme.bgColor);
        int bgG = GetGValue(currentTheme.bgColor);
        int bgB = GetBValue(currentTheme.bgColor);
        int shadowR = isNightMode ? 0 : 20;
        int shadowG = isNightMode ? 0 : 20;
        int shadowB = isNightMode ? 0 : 30;
        int mixR = (int)(bgR * (1 - alphaRatio) + shadowR * alphaRatio);
        int mixG = (int)(bgG * (1 - alphaRatio) + shadowG * alphaRatio);
        int mixB = (int)(bgB * (1 - alphaRatio) + shadowB * alphaRatio);
        setfillcolor(RGB(mixR, mixG, mixB));
        setlinecolor(RGB(mixR, mixG, mixB));
        solidcircle(CENTER_X, CENTER_Y, 8 + i);
    }
    // 绘制中心点主体
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
    
    // 初始化主题
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
            
            // 按钮点击
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
        
    // 小时变更：自动切主题
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