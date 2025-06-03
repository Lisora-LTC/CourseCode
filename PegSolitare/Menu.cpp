#include <iostream>
#include <easyx.h>
#include <tchar.h>
#include "Solitare.h"
#include <unordered_map> // 添加头文件
using namespace std;


int main() {
    init();

    // 使用枚举变量管理当前状态
    GameState currentState = MAIN_MENU;

    // 渲染主菜单页面
    mainMenu.render();

    while (currentState != EXIT) {
        // 检查鼠标左键是否被按下
        if (!GetAsyncKeyState(VK_LBUTTON)) {
            Sleep(50); // 等待事件
            continue; // 跳过本次循环
        }

        // 根据 currentState 获取对应的页面对象
        StateNode* currentNode = stateMap[currentState];
        GameState previousState = currentState;

        currentNode->handleEvent(currentState); // 处理事件并切换状态

        // 只有页面状态发生变化时才清屏并重新渲染
        if (currentState != previousState) {
            currentNode = stateMap[currentState];
            currentNode->render(); // 渲染当前页面
        }//加一个exit窗口提问是否退出
    }

    closegraph(); // 关闭图形界面
    return 0;
}

void init(){ //初始化图形界面

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
        
}