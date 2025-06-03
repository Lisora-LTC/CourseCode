#include <vector>
#include <easyx.h>
#include <tchar.h>
#include "Solitare.h"
#include <unordered_map>

using namespace std;

MainMenuState mainMenu;
ChooseGameState chooseGame;
ExitState exitState;

// MainMenuState
void MainMenuState::render() {
    cleardevice(); // 清屏
    // 顶部深蓝条
    setfillcolor(RGB(0, 84, 153));
    solidrectangle(0, 0, 1280, 120);
    // 页面标题
    pageTitle.draw();
    // 按钮
    startButton.draw();
    exitButton.draw();
}

StateNode* MainMenuState::handleEvent() {
    if (GetAsyncKeyState(VK_LBUTTON)) {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(GetForegroundWindow(), &pt);
        if (startButton.isClicked(pt.x, pt.y)) {
            return &chooseGame;
        } else if (exitButton.isClicked(pt.x, pt.y)) {
            return &exitState;
        }
    }
    return this;
}

// ChooseGameState
void ChooseGameState::render() {
    cleardevice(); // 清屏
    // 顶部深蓝条
    setfillcolor(RGB(0, 84, 153));
    solidrectangle(0, 0, 1280, 120);
    // 页面标题
    pageTitle.draw();
    // 返回按钮
    returnButton.draw();
}

StateNode* ChooseGameState::handleEvent() {
    if (GetAsyncKeyState(VK_LBUTTON)) {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(GetForegroundWindow(), &pt);
        if (returnButton.isClicked(pt.x, pt.y)) {
            return &mainMenu;
        }
    }
    return this;
}

// ExitState
void ExitState::render() {
    // 在屏幕中央弹出白色对话框
    setfillcolor(WHITE);
    solidrectangle(440, 260, 840, 460);
    setlinecolor(RGB(0, 84, 153));
    rectangle(440, 260, 840, 460);

    settextstyle(28, 0, _T("微软雅黑"));
    settextcolor(RGB(0, 84, 153));
    setbkmode(TRANSPARENT);
    const TCHAR* prompt = _T("是否退出游戏？");
    int w = textwidth(prompt);
    outtextxy(1280/2 - w/2, 300, prompt);

    // 绘制按钮
    yesButton.draw();
    noButton.draw();
}

StateNode* ExitState::handleEvent() {
    if (GetAsyncKeyState(VK_LBUTTON)) {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(GetForegroundWindow(), &pt);

        if (yesButton.isClicked(pt.x, pt.y)) {
            return nullptr; // 结束循环退出程序
        } else if (noButton.isClicked(pt.x, pt.y)) {
            return &mainMenu; // 返回主菜单
        }
    }
    return this;
}
