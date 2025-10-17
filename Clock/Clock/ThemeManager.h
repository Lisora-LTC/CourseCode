#pragma once
#include <graphics.h>

// 主题结构体（配色）
struct Theme {
    COLORREF bgColor;
    COLORREF dialCenterColor;
    COLORREF dialEdgeColor;
    COLORREF borderColor;
    COLORREF hourHandColor;
    COLORREF minuteHandColor;
    COLORREF secondHandColor;
    COLORREF textColor;
    COLORREF centerDotColor;
};

// 预设主题
extern const Theme DAY_THEME;    // 日间
extern const Theme NIGHT_THEME;  // 夜间

// 当前主题
extern Theme currentTheme;

// 按小时切换主题
void updateTheme(int hour);
