#include "Solitare.h"
#include <tchar.h>
//#include <graphics.h>
#include <easyx.h>

Button::Button(int x, int y, int width, int height, const TCHAR* text, 
               COLORREF fill, COLORREF border, COLORREF textCol)
    : x(x), y(y), width(width), height(height), text(text), 
      fillColor(fill), borderColor(border), textColor(textCol) {}

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
