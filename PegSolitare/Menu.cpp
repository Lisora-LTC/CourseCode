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

// 游戏状态专用循环，直到退出游戏状态
StateNode* runGameLoop(GameState* gs) {
    // 首次渲染
    gs->render();
    bool lastPressed = false;
    POINT lastMousePos = {-1, -1}; // 记录上次鼠标位置
    bool needsRender = false;
    
    while (true) {
        // 获取当前鼠标位置
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);
        ScreenToClient(GetForegroundWindow(), &currentMousePos);
        
        // 检测鼠标位置是否改变（用于悬停效果）
        if (currentMousePos.x != lastMousePos.x || currentMousePos.y != lastMousePos.y) {
            needsRender = true;
            lastMousePos = currentMousePos;
        }        // 检测鼠标点击
        bool currentPressed = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
        if (currentPressed && !lastPressed) {
            StateNode* next = gs->handleEvent();
            if (next != gs) return next;
            needsRender = true; // 点击后可能状态改变，需要渲染
        }
        
        // 只在需要时渲染
        if (needsRender) {
            gs->render();
            FlushBatchDraw(); // 将后台缓冲区内容显示到屏幕
            needsRender = false;
        }
        
        lastPressed = currentPressed;
        Sleep(16); // 保持游戏的流畅性
    }
}

int main() {
    init();
    
    StateNode* current = &mainMenu;
    current->render();
    FlushBatchDraw(); // 初始渲染后刷新显示
    
    bool lastPressed = false; // 记录上一次鼠标按键状态
    bool needsRender = false; // 标记是否需要重新渲染

    while (current) {        // 如果进入游戏状态，使用专用循环
        if (current == &gameState) {
            current = runGameLoop(static_cast<GameState*>(current));
            if (current) {
                current->render();
                FlushBatchDraw(); // 刷新显示
            }
            lastPressed = true; // 从游戏循环退出时也要重置鼠标状态，防止连续触发
            continue;
        }
        
        bool currentPressed = GetAsyncKeyState(VK_LBUTTON) & 0x8000; // 当前鼠标状态
          // 检测鼠标点击事件（从未按下到按下的边沿触发）
        if (currentPressed && !lastPressed) {
            StateNode* next = current->handleEvent();
            if (next != current) {
                current = next;
                needsRender = true; // 状态改变时才需要渲染
                lastPressed = true; // 状态切换时重置鼠标状态，防止连续触发
            }
        }// 只在需要时渲染（状态改变时）
        if (needsRender && current) {
            current->render();
            FlushBatchDraw(); // 将后台缓冲区内容显示到屏幕
            needsRender = false;
        }
        
        lastPressed = currentPressed;
        Sleep(50); // 降低非游戏界面的刷新频率
    }

    EndBatchDraw(); // 结束双缓冲
    closegraph(); // 关闭图形界面
    return 0;
}