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
    settextstyle(60, 0, _T("Arial"));
    settextcolor(BLACK);
    outtextxy(1280 / 2 - textwidth(_T("孔明棋")) / 2, 100, _T("孔明棋"));
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
    settextstyle(60, 0, _T("Arial"));
    settextcolor(BLACK);
    outtextxy(1280 / 2 - textwidth(_T("选择游戏")) / 2, 100, _T("选择游戏"));
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
    // 在屏幕中央弹出一个窗口
    setfillcolor(LIGHTGRAY);
    solidrectangle(440, 260, 840, 460); // 窗口大小

    settextstyle(30, 0, _T("Arial"));
    settextcolor(BLACK);
    outtextxy(1280 / 2 - textwidth(_T("是否退出游戏？")) / 2, 300, _T("是否退出游戏？"));

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
