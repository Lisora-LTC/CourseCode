#include <iostream>
#include <easyx.h>
#include <tchar.h>
#include <windows.h>  // 添加Windows头文件以支持DPI函数
#include "Solitare.h"
using namespace std;

void init(){ // 初始化图形界面
    
    // 获取屏幕分辨率
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);        // 计算窗口位置，使其居中 (720p)
        int windowWidth = 1280;
        int windowHeight = 720;
        int windowX = (screenWidth - windowWidth) / 2;
        int windowY = (screenHeight - windowHeight) / 2;        // 设置窗口位置并初始化图形窗口
        HWND hwnd = initgraph(windowWidth, windowHeight);
        SetWindowPos(hwnd, HWND_TOP, windowX, windowY, windowWidth, windowHeight, SWP_SHOWWINDOW);        
        
        setbkcolor(RGB(240, 248, 255)); // 浅蓝背景 (AliceBlue)
        cleardevice();
        
        // 启用双缓冲，消除闪烁
        BeginBatchDraw();
}

int main() {
    init();
    
    StateNode* current = &mainMenu;
    current->render();
    FlushBatchDraw(); // 初始渲染后刷新显示
    
    bool lastPressed = false; // 记录上一次鼠标按键状态
    POINT lastMousePos = {-1, -1}; // 记录上次鼠标位置
    bool needsRender = false; // 标记是否需要重新渲染

    while (current) {
        // 获取当前鼠标位置
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);
        ScreenToClient(GetForegroundWindow(), &currentMousePos);
          // 检测鼠标位置是否改变（用于悬停效果和实时更新）
        // 对于游戏状态，减少重新渲染频率以提高性能
        if (currentMousePos.x != lastMousePos.x || currentMousePos.y != lastMousePos.y) {
            if (current != &gameState) {
                // 非游戏界面：立即重新渲染（按钮悬停效果）
                needsRender = true;
            } else {
                // 游戏界面：只在鼠标位置变化较大时重新渲染
                int deltaX = abs(currentMousePos.x - lastMousePos.x);
                int deltaY = abs(currentMousePos.y - lastMousePos.y);
                if (deltaX > 5 || deltaY > 5) {  // 减少敏感度
                    needsRender = true;
                }
            }
            lastMousePos = currentMousePos;
        }
        
        bool currentPressed = GetAsyncKeyState(VK_LBUTTON) & 0x8000; // 当前鼠标状态
        
        // 检测鼠标点击事件（从未按下到按下的边沿触发）
        if (currentPressed && !lastPressed) {
            StateNode* next = current->handleEvent();
            // 游戏状态下点击也需要重绘，即使未切换状态
            if (next != current || current == &gameState) {
                current = next;
                needsRender = true;
            }
        }
        
        // 只在需要时渲染（状态改变时或鼠标位置改变时）
        if (needsRender && current) {
            current->render();
            FlushBatchDraw(); // 将后台缓冲区内容显示到屏幕
            needsRender = false;
        }
        
        lastPressed = currentPressed;
        Sleep(16); // 保持游戏的流畅性，约60fps
    }

    EndBatchDraw(); // 结束双缓冲
    closegraph(); // 关闭图形界面
    return 0;
}