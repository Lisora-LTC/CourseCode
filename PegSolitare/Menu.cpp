#include <iostream>
#include <easyx.h>
#include <tchar.h>
#include "Solitare.h"
using namespace std;


int main() {
    init();
    mainMenu();
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

void mainMenu(){ //渲染主菜单
    // 设置标题
    settextstyle(60, 0, _T("Arial")); // 增大标题文字大小
    settextcolor(BLACK); // 字体颜色改为黑色
    outtextxy(1280 / 2 - textwidth(_T("孔明棋")) / 2, 100, _T("孔明棋"));

    // 创建按钮对象
    Button startButton(560, 300, 160, 50, _T("开始游戏"));
    Button exitButton(560, 400, 160, 50, _T("退出游戏"));

    // 绘制按钮
    startButton.draw();
    exitButton.draw();

    while (true) {
        // 检测鼠标点击事件
        if (GetAsyncKeyState(VK_LBUTTON)) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(GetForegroundWindow(), &pt);
            //checkExitButton(pt.x, pt.y); // 检测是否点击了“退出游戏”按钮
            if(startButton.isClicked(pt.x, pt.y)){
                chooseGame();
            }
            if(exitButton.isClicked(pt.x, pt.y)){
                closegraph(); // 关闭图形化窗口
                exit(0); // 直接终止程序运行
            }
        }
        Sleep(100); // 等待事件
    }
    return;
}

void chooseGame(){
    cleardevice(); // 清屏，避免页面组件遮挡

    // 设置标题
    settextstyle(60, 0, _T("Arial")); // 增大标题文字大小
    settextcolor(BLACK); // 字体颜色改为黑色
    outtextxy(1280 / 2 - textwidth(_T("选择游戏")) / 2, 100, _T("选择游戏"));

    // 创建返回按钮
    Button returnButton(20, 20, 100, 40, _T("返回"));

    // 绘制返回按钮
    returnButton.draw();

    while (true) {
        // 检测鼠标点击事件
        if (GetAsyncKeyState(VK_LBUTTON)) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(GetForegroundWindow(), &pt);
            if (returnButton.isClicked(pt.x, pt.y)) {
                cleardevice();
                mainMenu();
                return;
            }
        }
        Sleep(100); // 等待事件
    }
}