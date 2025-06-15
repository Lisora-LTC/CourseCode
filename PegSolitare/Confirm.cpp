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
    // 全屏背景遮蔽
    setfillcolor(backgroundColor);
    solidrectangle(0, 0, 1280, 720);
    
    // 在屏幕中央弹出对话框 (720p适配)
    setfillcolor(dialogColor);
    setlinecolor(borderColor);
    setlinestyle(PS_SOLID, 2);
    fillroundrect(390, 260, 890, 460, 10, 10);  // 圆角矩形
    
    // 设置抗锯齿字体
    LOGFONT font;
    gettextstyle(&font);
    font.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    setbkmode(TRANSPARENT);
    
    // 渲染标题（如果有）
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
    
    // 渲染提示文字（如果有）
    if (hintText && _tcslen(hintText) > 0) {
        font.lfHeight = 20;
        settextstyle(&font);
        settextcolor(hintColor);
        int hintWidth = textwidth(hintText);
        outtextxy(640 - hintWidth/2, titleText ? 370 : 360, hintText);
    }
      // 获取当前鼠标位置
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    // 绘制按钮（带悬停效果）
    yesButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    noButton.drawWithHover(currentMousePos.x, currentMousePos.y);
}

// ConfirmBase 不再处理事件，只负责渲染
// 每个子类自己实现 handleEvent 方法

// 确认相关状态类的全局对象实例
ExitState exitState;
ContinueGameState continueGameState;
GameFailedState gameFailedState;
GameWonState gameWonState;
RestartConfirmState restartConfirmState;

// ExitState 重构为使用ConfirmBase
ExitState::ExitState() : ConfirmBase(
    nullptr,                    // 无标题
    _T("是否退出游戏？"),        // 正文内容
    nullptr,                    // 无提示文字
    RGB(240, 245, 250),        // 浅蓝白色背景
    WHITE,                     // 白色对话框
    RGB(0, 84, 153),          // 蓝色边框
    RGB(0, 84, 153),          // 蓝色标题
    RGB(0, 84, 153),          // 蓝色内容
    RGB(128, 128, 128),       // 灰色提示
    RGB(0, 120, 215),         // 蓝色确认按钮
    RGB(0, 120, 215)          // 蓝色取消按钮
) {}

StateNode* ExitState::handleEvent() {
    // 获取鼠标点击位置
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    // 直接处理按钮点击
    if (yesButton.isClicked(pt.x, pt.y)) {
        return nullptr; // 退出程序
    } else if (noButton.isClicked(pt.x, pt.y)) {
        return &mainMenu; // 返回主菜单
    }
    return this;
}

// ContinueGameState 重构为使用ConfirmBase
ContinueGameState::ContinueGameState() : ConfirmBase(
    nullptr,                    // 无标题
    _T("是否继续上次的游戏？"), // 正文内容
    nullptr,                    // 无提示文字
    RGB(240, 245, 250),        // 浅蓝白色背景
    WHITE,                     // 白色对话框
    RGB(0, 84, 153),          // 蓝色边框
    RGB(0, 84, 153),          // 蓝色标题
    RGB(0, 84, 153),          // 蓝色内容
    RGB(128, 128, 128),       // 灰色提示
    RGB(0, 120, 215),         // 蓝色确认按钮
    RGB(0, 120, 215)          // 蓝色取消按钮
) {}

StateNode* ContinueGameState::handleEvent() {
    // 获取鼠标点击位置
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    // 直接处理按钮点击
    if (yesButton.isClicked(pt.x, pt.y)) {
        return &gameState; // 继续上次的游戏
    } else if (noButton.isClicked(pt.x, pt.y)) {
        gameState.resetGame(); // 重置游戏状态
        // 检查是否要开始残局模式
        if (chooseGame.isPendingEndgame()) {
            gameState.startEndgame();
            chooseGame.setPendingEndgame(false); // 清除标记
        }
        return &gameState; // 开始新游戏或残局
    }
    return this;
}

// RestartConfirmState 重构为使用ConfirmBase
RestartConfirmState::RestartConfirmState() : ConfirmBase(
    _T("重新开始"),              // 标题
    _T("是否重新开始当前游戏？"), // 正文内容
    _T("当前进度将会丢失"),       // 提示文字
    RGB(255, 245, 235),        // 浅橙色背景
    RGB(255, 248, 240),        // 橙白色对话框
    RGB(255, 140, 0),          // 橙色边框
    RGB(255, 140, 0),          // 橙色标题
    RGB(102, 51, 0),           // 深橙棕色内容
    RGB(153, 102, 51),         // 中等橙棕色提示
    RGB(255, 140, 0),          // 橙色确认按钮
    RGB(255, 165, 0)           // 浅橙色取消按钮
) {}

StateNode* RestartConfirmState::handleEvent() {
    // 获取鼠标点击位置
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    // 直接处理按钮点击
    if (yesButton.isClicked(pt.x, pt.y)) {
        gameState.restartCurrentMode(); // 重新开始当前模式
        return &gameState;
    } else if (noButton.isClicked(pt.x, pt.y)) {
        return &gameState; // 返回游戏界面
    }
    return this;
}

// GameFailedState 重构为使用ConfirmBase
GameFailedState::GameFailedState() : ConfirmBase(
    _T("游戏结束"),              // 标题
    _T("已无可移动的棋子！"),     // 正文内容
    _T("别灰心！孔明棋需要策略和耐心"), // 提示文字
    RGB(245, 242, 238),        // 浅暖白色背景
    RGB(250, 240, 230),        // 温暖的米白色对话框
    RGB(220, 100, 80),         // 温和的橙红色边框
    RGB(220, 100, 80),         // 温和的橙红色标题
    RGB(80, 80, 80),           // 深灰色内容
    RGB(120, 120, 120),        // 浅灰色提示
    RGB(220, 100, 80),         // 橙红色确认按钮
    RGB(220, 100, 80),         // 橙红色取消按钮
    _T("悔棋"),                 // 确认按钮文字
    _T("退出")                  // 取消按钮文字
) {}

StateNode* GameFailedState::handleEvent() {
    // 获取鼠标点击位置
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    // 直接处理按钮点击
    if (yesButton.isClicked(pt.x, pt.y)) {
        // 直接悔棋，无需确认
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

// GameWonState 重构为使用ConfirmBase
GameWonState::GameWonState() : ConfirmBase(
    _T("恭喜胜利！"),            // 标题
    _T("您成功完成了孔明棋！"),   // 正文内容
    _T("智慧与策略的完美结合"),   // 提示文字
    RGB(250, 248, 240),        // 浅金白色背景
    RGB(255, 248, 220),        // 淡金色对话框
    RGB(255, 215, 0),          // 金色边框
    RGB(255, 165, 0),          // 橙金色标题
    RGB(80, 80, 80),           // 深灰色内容
    RGB(120, 120, 120),        // 浅灰色提示
    RGB(255, 215, 0),          // 金色确认按钮
    RGB(255, 215, 0),          // 金色取消按钮
    _T("再来一局"),             // 确认按钮文字
    _T("退出")                  // 取消按钮文字
) {}


StateNode* GameWonState::handleEvent() {
    // 获取鼠标点击位置
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    // 直接处理按钮点击
    if (yesButton.isClicked(pt.x, pt.y)) {
        // 检查当前游戏模式，开始相应的下一局
        if (gameState.isEndgameMode()) {
            // 当前是残局模式，开始新的残局
            gameState.startEndgame();
        } else {
            // 当前是经典模式，重置并开始经典模式
            gameState.resetGame();
        }
        return &gameState;
    } else if (noButton.isClicked(pt.x, pt.y)) {
        // 重置游戏并返回选择游戏界面
        gameState.resetGame();
        return &chooseGame;
    }
    return this;
}


