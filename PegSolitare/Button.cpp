#include "Solitare.h"
#include <tchar.h>
#include <easyx.h>

Button::Button(int x, int y, int width, int height, const TCHAR* text, 
               COLORREF fill, COLORREF border, COLORREF textCol, bool isEnabled)
    : x(x), y(y), width(width), height(height), text(text), 
      fillColor(fill), borderColor(border), textColor(textCol), enabled(isEnabled) {}

void Button::draw() const {
    setfillcolor(fillColor);
    solidrectangle(x, y, x + width, y + height);
    setlinecolor(borderColor);
    rectangle(x, y, x + width, y + height);
    
    // 启用抗锯齿字体渲染
    LOGFONT font;
    gettextstyle(&font);
    font.lfHeight = 28;
    font.lfWidth = 0;
    font.lfWeight = FW_NORMAL;
    font.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    settextstyle(&font);
    
    settextcolor(textColor);
    setbkmode(TRANSPARENT);
    outtextxy(x + width / 2 - textwidth(text) / 2, 
              y + height / 2 - textheight(text) / 2, text);
}

bool Button::isClicked(int mouseX, int mouseY) const {
    return (mouseX >= x && mouseX <= x + width &&
            mouseY >= y && mouseY <= y + height);
}

void Button::drawWithHover(int mouseX, int mouseY) const {
    bool hovered = enabled && isHovered(mouseX, mouseY);
    
    setfillcolor(fillColor);
    solidrectangle(x, y, x + width, y + height);
    
    setlinecolor(borderColor);
    rectangle(x, y, x + width, y + height);    
    if (hovered) {
        // 计算浅色版本的边框颜色
        int r = GetRValue(borderColor);
        int g = GetGValue(borderColor);
        int b = GetBValue(borderColor);
        
        r = min(255, r + 80);
        g = min(255, g + 80);
        b = min(255, b + 80);
        
        COLORREF hoverColor = RGB(r, g, b);
        setlinecolor(hoverColor);
        setlinestyle(PS_SOLID, 3);
        
        rectangle(x - 2, y - 2, x + width + 2, y + height + 2);
        
        setlinestyle(PS_SOLID, 1);
    }
    
    // 启用抗锯齿字体渲染
    LOGFONT font;
    gettextstyle(&font);
    font.lfHeight = 28;
    font.lfWidth = 0;
    font.lfWeight = FW_NORMAL;
    font.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    settextstyle(&font);
    
    settextcolor(textColor);
    setbkmode(TRANSPARENT);
    outtextxy(x + width / 2 - textwidth(text) / 2, 
              y + height / 2 - textheight(text) / 2, text);
}

bool Button::isHovered(int mouseX, int mouseY) const {
    return (mouseX >= x && mouseX <= x + width &&
            mouseY >= y && mouseY <= y + height);
}

void Button::setEnabled(bool isEnabled) {
    enabled = isEnabled;
}

bool Button::getEnabled() const {
    return enabled;
}
