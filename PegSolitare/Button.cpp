#include "Solitare.h"
#include <tchar.h>
//#include <graphics.h>
#include <easyx.h>

Button::Button(int x, int y, int width, int height, const TCHAR* text)
    : x(x), y(y), width(width), height(height), text(text) {}

void Button::draw() const {
    // 蓝底白字按钮，带深蓝边框
    setfillcolor(RGB(0, 120, 215));
    solidrectangle(x, y, x + width, y + height);
    setlinecolor(RGB(0, 84, 153));
    rectangle(x, y, x + width, y + height);
    settextstyle(28, 0, _T("微软雅黑")); // 使用微软雅黑字体
    settextcolor(WHITE); // 字体颜色为白色
    setbkmode(TRANSPARENT); // 文字背景透明
    outtextxy(x + width / 2 - textwidth(text) / 2, y + height / 2 - textheight(text) / 2, text);
}

bool Button::isClicked(int mouseX, int mouseY) const {
    // 检测鼠标点击是否在按钮范围内
    return (mouseX >= x && mouseX <= x + width &&
            mouseY >= y && mouseY <= y + height);
}
