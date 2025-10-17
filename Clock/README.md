2451367 李天成

这是一个基于 EasyX 图形库的时钟程序，支持日夜主题切换和暗角效果。

基础功能：时钟指针采用类进行继承，指针类 ClockHand支持设置中心/长度/角度/颜色/粗细。提高资源利用率。下面有数字时钟，方便快速确认当前时间，以及日期，星期等。时钟会自动根据日夜状况调整昼夜模式，更加护眼。

最终效果如下图：

![image-20251008124627887](C:\Users\Lenovo\AppData\Roaming\Typora\typora-user-images\image-20251008124627887.png)

1.抗锯齿模块(AntiAlias.h/.cpp)

起初我采用Wu反走样算法（Xiaolin Wu’s Algorithm）来实现抗锯齿，但很遗憾该算法对曲线抗锯齿的效果不是太好，故最终决定采用2xSSAA算法来实现抗锯齿。SSAA的核心原理是将场景渲染到更高分辨率的缓冲区（例如4xSSAA会将800×600分辨率渲染到1600×1200缓冲区），再通过下采样平均混合相邻像素颜色，生成最终图像。针对每一个子采样点，重复母像素的流程，判定是否在三角形内、深度检测、着色，最后每个子采样点都会得到各自的颜色，母像素的颜色就是子采样点颜色的平均。将4个子采样点的颜色平均后，三角形边缘的有些像素点会变淡，从宏观上看，锯齿就不那么明显。

同时出于对在配置较低的电脑上也能流畅运行的考虑，我在代码中保留了一个接口，可以分别控制动态元素（指针）与静态元素（表盘与背景等）的抗锯齿算法的选择。接口如下。

```c++
// 设置为 1 使用 2xSSAA，0 使用 Wu's 算法

#define USE_2X_SSAA 1
#define USE_2X_SSAA_FOR_STATIC 1  

// 用于动态元素（指针）
#if USE_2X_SSAA
    #define AA_DrawLine(x1, y1, x2, y2, color, thickness) \
        AntiAlias::SSAA::drawLine2xSSAA(x1, y1, x2, y2, color, thickness, WIN_W, WIN_H)
    #define AA_DrawCircle(cx, cy, r, color, filled) \
        AntiAlias::SSAA::drawCircle2xSSAA(cx, cy, r, color, filled, WIN_W, WIN_H)
#else //静态元素
    #define AA_DrawLine(x1, y1, x2, y2, color, thickness) \
        AntiAlias::drawAntiAliasedLine(x1, y1, x2, y2, color, thickness, WIN_W, WIN_H)
    #define AA_DrawCircle(cx, cy, r, color, filled) \
        AntiAlias::drawAntiAliasedCircle(cx, cy, r, color, filled, WIN_W, WIN_H)
#endif
```

2.日夜主题切换(ThemeManager.h/.cpp)

考虑到同学们总是喜欢熬夜，为了保护大家的视力，本时钟会自动根据昼（6:00 - 18:00）夜（18:00 - 6:00）时间选择对应模式。同时程序右上角保留了主动切换昼夜模式的按钮，分别有自动，昼，夜三种模式。

```c++
const Theme DAY_THEME = {
    RGB(239, 246, 255),     // bgColor - 柔和浅蓝背景
    RGB(255, 255, 255),     // dialCenterColor - 纯白中心
    RGB(245, 247, 250),     // dialEdgeColor - 柔和灰白边缘
    RGB(70, 100, 140),      // borderColor - 深蓝边框
    RGB(15, 35, 80),        // hourHandColor - 深蓝时针
    RGB(30, 80, 180),       // minuteHandColor - 海蓝分针
    RGB(220, 38, 127),      // secondHandColor - 粉红秒针
    RGB(15, 35, 80),        // textColor - 深蓝文字
    RGB(15, 35, 80)         // centerDotColor - 深蓝中心点
};

//  夜间模式主题（18:00 - 6:00）
const Theme NIGHT_THEME = {
    RGB(20, 25, 35),        // bgColor - 深蓝黑背景
    RGB(45, 52, 65),        // dialCenterColor - 深灰蓝中心
    RGB(30, 35, 45),        // dialEdgeColor - 更深灰蓝边缘
    RGB(80, 100, 130),      // borderColor - 柔和蓝边框
    RGB(200, 210, 230),     // hourHandColor - 浅灰白时针
    RGB(120, 180, 255),     // minuteHandColor - 亮蓝分针
    RGB(255, 100, 150),     // secondHandColor - 亮粉秒针
    RGB(180, 190, 210),     // textColor - 浅灰文字
    RGB(120, 180, 255)      // centerDotColor - 亮蓝中心点
};
```

3.增量重绘，仅回填指针覆盖区域

每一帧只重绘“会变的区域”和“必须覆盖的层”，避免整屏清空与全量绘制，从而减少闪烁、降低开销，同时保证视觉一致性。

```c++
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
```

4.其他视觉效果：

(1)暗角效果 applyVignette（整体氛围）

整个程序背景的外围做了一圈灰色暗角效果，视觉上突出了中间的时钟，让时钟更有立体感，更加高级。

```c++
void applyVignette() {
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
    
    //  单独绘制四个角落的渐变（对角线方向）
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
```

(2)背景微光圈层（柔化背景）

柔化背景的本质是几圈同心、逐层变淡的提亮实心圆。它能制造轻微的景深和中心聚焦效果，让表盘更立体，更耐看。

```c++
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
```

(3)表盘径向渐变（中心亮→边缘暗）

表盘的中间比四周略微亮一点，使时钟更加立体，不那么扁平。

```c++
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
```

