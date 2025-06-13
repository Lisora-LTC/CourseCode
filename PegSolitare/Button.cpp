#include "Solitare.h"
#include <tchar.h>
//#include <graphics.h>
#include <easyx.h>

Button::Button(int x, int y, int width, int height, const TCHAR* text, 
               COLORREF fill, COLORREF border, COLORREF textCol, bool isEnabled)
    : x(x), y(y), width(width), height(height), text(text), 
      fillColor(fill), borderColor(border), textColor(textCol), enabled(isEnabled) {}

void Button::draw() const {
    // 使用传入的颜色绘制按钮
    setfillcolor(fillColor);
    solidrectangle(x, y, x + width, y + height);
    setlinecolor(borderColor);
    rectangle(x, y, x + width, y + height);
    
    // 启用抗锯齿字体渲染
    LOGFONT font;
    gettextstyle(&font);
    font.lfHeight = 28; // 恢复原始字体大小
    font.lfWidth = 0;
    font.lfWeight = FW_NORMAL;
    font.lfQuality = ANTIALIASED_QUALITY; // 启用抗锯齿
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    settextstyle(&font);
      settextcolor(textColor); // 使用传入的字体颜色
    setbkmode(TRANSPARENT); // 文字背景透明
    outtextxy(x + width / 2 - textwidth(text) / 2, 
              y + height / 2 - textheight(text) / 2, text);
}

bool Button::isClicked(int mouseX, int mouseY) const {
    // 检测鼠标点击是否在按钮范围内
    return (mouseX >= x && mouseX <= x + width &&
            mouseY >= y && mouseY <= y + height);
}

void Button::drawWithHover(int mouseX, int mouseY) const {
    bool hovered = enabled && isHovered(mouseX, mouseY); // 只有可按时才检测悬停
    
    // 使用传入的颜色绘制按钮
    setfillcolor(fillColor);
    solidrectangle(x, y, x + width, y + height);
    
    // 绘制边框
    setlinecolor(borderColor);
    rectangle(x, y, x + width, y + height);
    
    // 如果悬停且按钮可按，绘制浅色外围边框
    if (hovered) {
        // 计算浅色版本的边框颜色
        int r = GetRValue(borderColor);
        int g = GetGValue(borderColor);
        int b = GetBValue(borderColor);
        
        // 增加亮度，创建浅色效果
        r = min(255, r + 80);
        g = min(255, g + 80);
        b = min(255, b + 80);
        
        COLORREF hoverColor = RGB(r, g, b);
        setlinecolor(hoverColor);
        setlinestyle(PS_SOLID, 3); // 更粗的线条
        
        // 绘制外围边框（比按钮稍大）
        rectangle(x - 2, y - 2, x + width + 2, y + height + 2);
        
        // 恢复原始线条样式
        setlinestyle(PS_SOLID, 1);
    }
    
    // 启用抗锯齿字体渲染
    LOGFONT font;
    gettextstyle(&font);
    font.lfHeight = 28; // 恢复原始字体大小
    font.lfWidth = 0;
    font.lfWeight = FW_NORMAL;
    font.lfQuality = ANTIALIASED_QUALITY; // 启用抗锯齿
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    settextstyle(&font);
    
    settextcolor(textColor); // 使用传入的字体颜色
    setbkmode(TRANSPARENT); // 文字背景透明
    outtextxy(x + width / 2 - textwidth(text) / 2, 
              y + height / 2 - textheight(text) / 2, text);
}

bool Button::isHovered(int mouseX, int mouseY) const {
    // 检测鼠标是否悬停在按钮上（与isClicked相同的逻辑）
    return (mouseX >= x && mouseX <= x + width &&
            mouseY >= y && mouseY <= y + height);
}

void Button::setEnabled(bool isEnabled) {
    enabled = isEnabled;
}

bool Button::getEnabled() const {
    return enabled;
}
