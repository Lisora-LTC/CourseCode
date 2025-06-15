#include <vector>
#include <easyx.h>
#include <tchar.h>
#include "Solitare.h"
#include <unordered_map>
#include <cstdlib>
#include <ctime>

using namespace std;

// ConfirmBase 基类实现
ConfirmBase::ConfirmBase(
    const TCHAR* title, 
    const TCHAR* content, 
    const TCHAR* hint,
    COLORREF bgColor,
    COLORREF dlgColor,
    COLORREF borderClr,
    COLORREF titleClr,
    COLORREF contentClr,
    COLORREF hintClr,
    COLORREF yesClr,
    COLORREF noClr,
    const TCHAR* yesText,
    const TCHAR* noText
) : titleText(title), contentText(content), hintText(hint),
    backgroundColor(bgColor), dialogColor(dlgColor), borderColor(borderClr),
    titleColor(titleClr), contentColor(contentClr), hintColor(hintClr),
    yesButtonColor(yesClr), noButtonColor(noClr),
    yesButtonText(yesText), noButtonText(noText),
    yesButton(490, 400, 100, 40, yesText, yesClr, yesClr, WHITE),
    noButton(690, 400, 100, 40, noText, noClr, noClr, WHITE) {}

void ConfirmBase::render() {
    setfillcolor(backgroundColor);
    solidrectangle(0, 0, 1280, 720);
    
    setfillcolor(dialogColor);
    setlinecolor(borderColor);
    setlinestyle(PS_SOLID, 2);
    fillroundrect(390, 260, 890, 460, 10, 10);
    
    LOGFONT font;
    gettextstyle(&font);
    font.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    setbkmode(TRANSPARENT);
      // 渲染标题
    if (titleText && _tcslen(titleText) > 0) {
        font.lfHeight = 32;
        font.lfWeight = FW_BOLD;
        settextstyle(&font);
        settextcolor(titleColor);
        int titleWidth = textwidth(titleText);
        outtextxy(640 - titleWidth/2, 280, titleText);
    }
    
    // 渲染正文内容
    font.lfHeight = 28;
    font.lfWeight = FW_NORMAL;
    settextstyle(&font);
    settextcolor(contentColor);
    int contentWidth = textwidth(contentText);
    outtextxy(640 - contentWidth/2, titleText ? 330 : 320, contentText);
    
    // 渲染提示文字
    if (hintText && _tcslen(hintText) > 0) {
        font.lfHeight = 20;
        settextstyle(&font);
        settextcolor(hintColor);
        int hintWidth = textwidth(hintText);
        outtextxy(640 - hintWidth/2, titleText ? 370 : 360, hintText);
    }
    
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    yesButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    noButton.drawWithHover(currentMousePos.x, currentMousePos.y);
}

// 确认相关状态类的全局对象实例
ExitState exitState;
ContinueGameState continueGameState;
GameFailedState gameFailedState;
GameWonState gameWonState;
RestartConfirmState restartConfirmState;

// ExitState 实现
ExitState::ExitState() : ConfirmBase(
    nullptr,
    _T("是否退出游戏？"),
    nullptr,
    RGB(240, 245, 250),
    WHITE,
    RGB(0, 84, 153),
    RGB(0, 84, 153),
    RGB(0, 84, 153),
    RGB(128, 128, 128),
    RGB(0, 120, 215),
    RGB(0, 120, 215)
) {}

StateNode* ExitState::handleEvent() {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (yesButton.isClicked(pt.x, pt.y)) {
        return nullptr;
    } else if (noButton.isClicked(pt.x, pt.y)) {
        return &mainMenu;
    }
    return this;
}

// ContinueGameState 实现
ContinueGameState::ContinueGameState() : ConfirmBase(
    nullptr,
    _T("是否继续上次的游戏？"),
    nullptr,
    RGB(240, 245, 250),
    WHITE,
    RGB(0, 84, 153),
    RGB(0, 84, 153),
    RGB(0, 84, 153),
    RGB(128, 128, 128),
    RGB(0, 120, 215),
    RGB(0, 120, 215)
) {}

StateNode* ContinueGameState::handleEvent() {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (yesButton.isClicked(pt.x, pt.y)) {
        return &gameState;
    } else if (noButton.isClicked(pt.x, pt.y)) {
        gameState.resetGame();
        if (chooseGame.isPendingEndgame()) {
            gameState.startEndgame();
            chooseGame.setPendingEndgame(false);
        }
        return &gameState;
    }
    return this;
}

// RestartConfirmState 实现
RestartConfirmState::RestartConfirmState() : ConfirmBase(
    _T("重新开始"),
    _T("是否重新开始当前游戏？"),
    _T("当前进度将会丢失"),
    RGB(255, 245, 235),
    RGB(255, 248, 240),
    RGB(255, 140, 0),
    RGB(255, 140, 0),
    RGB(102, 51, 0),
    RGB(153, 102, 51),
    RGB(255, 140, 0),
    RGB(255, 165, 0)
) {}

StateNode* RestartConfirmState::handleEvent() {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (yesButton.isClicked(pt.x, pt.y)) {
        gameState.restartCurrentMode();
        return &gameState;
    } else if (noButton.isClicked(pt.x, pt.y)) {
        return &gameState;
    }
    return this;
}

// GameFailedState 实现
GameFailedState::GameFailedState() : ConfirmBase(
    _T("游戏结束"),
    _T("已无可移动的棋子！"),
    _T("别灰心！孔明棋需要策略和耐心"),
    RGB(245, 242, 238),
    RGB(250, 240, 230),
    RGB(220, 100, 80),
    RGB(220, 100, 80),
    RGB(80, 80, 80),
    RGB(120, 120, 120),
    RGB(220, 100, 80),
    RGB(220, 100, 80),
    _T("悔棋"),
    _T("退出")
) {}

StateNode* GameFailedState::handleEvent() {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (yesButton.isClicked(pt.x, pt.y)) {
        if (gameState.getBoard().canUndo()) {
            gameState.getBoard().undoMove();
        }
        return &gameState;
    } else if (noButton.isClicked(pt.x, pt.y)) {
        gameState.resetGame();
        return &chooseGame;
    }
    return this;
}

// GameWonState 实现
GameWonState::GameWonState() : ConfirmBase(
    _T("恭喜胜利！"),
    _T("您成功完成了孔明棋！"),
    _T("智慧与策略的完美结合"),
    RGB(250, 248, 240),
    RGB(255, 248, 220),
    RGB(255, 215, 0),
    RGB(255, 165, 0),
    RGB(80, 80, 80),
    RGB(120, 120, 120),
    RGB(255, 215, 0),
    RGB(255, 215, 0),
    _T("再来一局"),
    _T("退出")
) {}

StateNode* GameWonState::handleEvent() {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (yesButton.isClicked(pt.x, pt.y)) {
        if (gameState.isEndgameMode()) {
            gameState.startEndgame();
        } else {
            gameState.resetGame();
        }
        return &gameState;
    } else if (noButton.isClicked(pt.x, pt.y)) {
        gameState.resetGame();
        return &chooseGame;
    }
    return this;
}


