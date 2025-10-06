#include "ThemeManager.h"

// ☀️ 日间模式主题（6:00 - 18:00）
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

// 🌙 夜间模式主题（18:00 - 6:00）
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

// 当前激活的主题（全局变量，默认日间）
Theme currentTheme = DAY_THEME;

// 根据当前小时更新主题（仅更新配色，指针颜色由调用方更新）
void updateTheme(int hour) {
    // 判断日夜：6:00-17:59为日间，18:00-5:59为夜间
    if (hour >= 6 && hour < 18) {
        currentTheme = DAY_THEME;
    } else {
        currentTheme = NIGHT_THEME;
    }
}
