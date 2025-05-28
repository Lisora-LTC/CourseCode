#include <iostream>
#include <easyx.h>
#include <tchar.h>
using namespace std;

void drawButton(int x, int y, int width, int height, const TCHAR* text) {
    setfillcolor(RGB(255, 255, 0)); // 按钮颜色改为黄色
    solidrectangle(x, y, x + width, y + height);
    settextstyle(30, 0, _T("Arial")); // 调整按钮文字大小
    settextcolor(BLACK); // 字体颜色改为黑色
    outtextxy(x + width / 2 - textwidth(text) / 2, y + height / 2 - textheight(text) / 2, text);
}

void checkExitButton(int mouseX, int mouseY) {
    // 检测是否点击了“退出游戏”按钮
    int buttonX = 560, buttonY = 400, buttonWidth = 160, buttonHeight = 50;
    if (mouseX >= buttonX && mouseX <= buttonX + buttonWidth && mouseY >= buttonY && mouseY <= buttonY + buttonHeight) {
        closegraph();
        exit(0); // 结束程序
    }
}

int main() {
    // 获取屏幕分辨率
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 计算窗口位置，使其居中
    int windowWidth = 1280;
    int windowHeight = 720;
    int windowX = (screenWidth - windowWidth) / 2;
    int windowY = (screenHeight - windowHeight) / 2;

    // 设置窗口位置并初始化图形窗口
    HWND hwnd = initgraph(windowWidth, windowHeight);
    SetWindowPos(hwnd, HWND_TOP, windowX, windowY, windowWidth, windowHeight, SWP_SHOWWINDOW);

    setbkcolor(WHITE); // 背景颜色改为白色
    cleardevice();

    // 设置标题
    settextstyle(60, 0, _T("Arial")); // 增大标题文字大小
    settextcolor(BLACK); // 字体颜色改为黑色
    outtextxy(1280 / 2 - textwidth(_T("孔明棋")) / 2, 100, _T("孔明棋"));

    // 绘制按钮
    drawButton(560, 300, 160, 50, _T("开始游戏"));
    drawButton(560, 400, 160, 50, _T("退出游戏"));

    while (true) {
        // 检测鼠标点击事件
        if (GetAsyncKeyState(VK_LBUTTON)) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(GetForegroundWindow(), &pt);
            checkExitButton(pt.x, pt.y); // 检测是否点击了“退出游戏”按钮
        }
        Sleep(100); // 等待事件
    }

    closegraph();
    return 0;
}