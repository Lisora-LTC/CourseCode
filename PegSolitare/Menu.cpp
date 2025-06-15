#include <iostream>
#include <easyx.h>
#include <tchar.h>
#include <windows.h>
#include "Solitare.h"
using namespace std;

void init(){
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
    
    setbkcolor(RGB(240, 248, 255));
    cleardevice();
    
    // 启用双缓冲，消除闪烁
    BeginBatchDraw();
}

int main() {
    init();
    
    StateNode* current = &mainMenu;
    current->render();
    FlushBatchDraw();
    
    bool lastPressed = false;
    POINT lastMousePos = {-1, -1};
    bool needsRender = false;

    while (current) {
        // 获取当前鼠标位置
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);
        ScreenToClient(GetForegroundWindow(), &currentMousePos);
        
        // 检测鼠标位置是否改变
        if (currentMousePos.x != lastMousePos.x || currentMousePos.y != lastMousePos.y) {
            if (current != &gameState) {
                needsRender = true;
            } else {
                int deltaX = abs(currentMousePos.x - lastMousePos.x);
                int deltaY = abs(currentMousePos.y - lastMousePos.y);
                if (deltaX > 5 || deltaY > 5) {
                    needsRender = true;
                }
            }
            lastMousePos = currentMousePos;
        }
        
        bool currentPressed = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
        
        // 检测鼠标点击事件
        if (currentPressed && !lastPressed) {
            StateNode* next = current->handleEvent();
            if (next != current || current == &gameState) {
                current = next;
                needsRender = true;
            }
        }
        
        // 只在需要时渲染
        if (needsRender && current) {
            current->render();
            FlushBatchDraw();
            needsRender = false;
        }
        
        lastPressed = currentPressed;
        Sleep(16);
    }

    EndBatchDraw(); // 结束双缓冲
    closegraph(); // 关闭图形界面
    return 0;
}