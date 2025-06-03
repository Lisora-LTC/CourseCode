#include <vector>
#include <easyx.h>
#include <tchar.h>
#include "Solitare.h"
#include <unordered_map>

using namespace std;

// Global objects for states
MainMenuState mainMenu;
ChooseGameState chooseGame;
ExitState exitState;

// Global state map definition
unordered_map<GameState, StateNode*> stateMap = {
    {MAIN_MENU, &mainMenu},
    {CHOOSE_GAME, &chooseGame},
    {EXIT, &exitState}
};

// MainMenuState
void MainMenuState::render() {
    cleardevice(); // 清屏
    settextstyle(60, 0, _T("Arial"));
    settextcolor(BLACK);
    outtextxy(1280 / 2 - textwidth(_T("孔明棋")) / 2, 100, _T("孔明棋"));
    startButton.draw();
    exitButton.draw();
}

void MainMenuState::handleEvent(GameState& currentState) {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);

    if (startButton.isClicked(pt.x, pt.y)) {
        currentState = CHOOSE_GAME;
    } else if (exitButton.isClicked(pt.x, pt.y)) {
        currentState = EXIT;
    }
}

// ChooseGameState
void ChooseGameState::render() {
    cleardevice(); // 清屏
    settextstyle(60, 0, _T("Arial"));
    settextcolor(BLACK);
    outtextxy(1280 / 2 - textwidth(_T("选择游戏")) / 2, 100, _T("选择游戏"));
    returnButton.draw();
}

void ChooseGameState::handleEvent(GameState& currentState) {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);

    if (returnButton.isClicked(pt.x, pt.y)) {
        currentState = MAIN_MENU;
    }
}

// ExitState
void ExitState::render() {
    // 在屏幕中央弹出一个窗口
    setfillcolor(WHITE);
    solidrectangle(480, 260, 800, 460); // 窗口大小

    settextstyle(30, 0, _T("Arial"));
    settextcolor(BLACK);
    outtextxy(560, 300, _T("是否退出游戏？"));

    // 绘制按钮
    yesButton.draw();
    noButton.draw();
}

void ExitState::handleEvent(GameState& currentState) {
    if (GetAsyncKeyState(VK_LBUTTON)) {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(GetForegroundWindow(), &pt);

        if (yesButton.isClicked(pt.x, pt.y)) {
            currentState = EXIT; // 退出游戏
        } else if (noButton.isClicked(pt.x, pt.y)) {
            currentState = MAIN_MENU; // 返回主菜单
            mainMenu.render();
        }
    }
};
