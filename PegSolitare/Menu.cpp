#include <iostream>
#include <easyx.h>
#include <tchar.h>
#include "Solitare.h"
using namespace std;

int main() {
    init();

    StateNode* current = &mainMenu;
    current->render();

    while (current) {
        if (!GetAsyncKeyState(VK_LBUTTON)) {// 改用WM_LBUTTONDOWN——鼠标左键按下？
            Sleep(50);
            continue;
        }

        StateNode* next = current->handleEvent();
        if (next != current) {
            current = next;
            if (current) current->render();
        }
    }

    closegraph(); // 关闭图形界面
    return 0;
}

void init(){ // 初始化图形界面

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

        setbkcolor(RGB(240, 248, 255)); // 浅蓝背景 (AliceBlue)
        cleardevice();
        
}