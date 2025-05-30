#include "Solitare.h"
#include <tchar.h>
//#include <graphics.h>
#include <easyx.h>

Button::Button(int x, int y, int width, int height, const TCHAR* text)
    : x(x), y(y), width(width), height(height), text(text) {}

void Button::draw() const {
    setfillcolor(RGB(255, 255, 0)); // 按钮颜色改为黄色
    solidrectangle(x, y, x + width, y + height);
    settextstyle(30, 0, _T("Arial")); // 调整按钮文字大小
    settextcolor(BLACK); // 字体颜色改为黑色
    outtextxy(x + width / 2 - textwidth(text) / 2, y + height / 2 - textheight(text) / 2, text);
}

bool Button::isClicked(int mouseX, int mouseY) const {
    // 检测鼠标点击是否在按钮范围内
    return (mouseX >= x && mouseX <= x + width &&
            mouseY >= y && mouseY <= y + height);
}
