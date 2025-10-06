#pragma once
#include <graphics.h>

// ========== 日夜模式主题系统 ==========
// 主题结构体：存储所有配色
struct Theme {
    COLORREF bgColor;           // 背景色
    COLORREF dialCenterColor;   // 表盘中心色（渐变亮端）
    COLORREF dialEdgeColor;     // 表盘边缘色（渐变暗端）
    COLORREF borderColor;       // 表盘边框色
    COLORREF hourHandColor;     // 时针颜色
    COLORREF minuteHandColor;   // 分针颜色
    COLORREF secondHandColor;   // 秒针颜色
    COLORREF textColor;         // 数字/文字颜色
    COLORREF centerDotColor;    // 中心装饰点颜色
};

// ☀️ 日间模式主题（6:00 - 18:00）
extern const Theme DAY_THEME;

// 🌙 夜间模式主题（18:00 - 6:00）
extern const Theme NIGHT_THEME;

// 当前激活的主题（全局变量）
extern Theme currentTheme;

// 根据当前小时更新主题
void updateTheme(int hour);
