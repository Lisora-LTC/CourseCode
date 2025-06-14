#include <vector>
#include <easyx.h>
#include <tchar.h>
#include "Solitare.h"
#include <unordered_map>
#include <cstdlib>
#include <ctime>

using namespace std;

// 确认相关状态类的全局对象实例
ExitState exitState;
ContinueGameState continueGameState;
GameFailedState gameFailedState;
GameWonState gameWonState;
RestartConfirmState restartConfirmState;

// ExitState 实现
void ExitState::render() {
    // 全屏背景遮蔽
    setfillcolor(RGB(240, 245, 250)); // 浅蓝白色背景
    solidrectangle(0, 0, 1280, 720);
    
    // 在屏幕中央弹出白色对话框 (720p适配)
    setfillcolor(WHITE);
    setlinecolor(RGB(0, 84, 153));
    setlinestyle(PS_SOLID, 2);
    fillroundrect(390, 260, 890, 460, 10, 10);  // 圆角矩形
    
    // 启用抗锯齿字体渲染
    LOGFONT font;
    gettextstyle(&font);
    font.lfHeight = 28;
    font.lfWidth = 0;
    font.lfWeight = FW_NORMAL;
    font.lfQuality = ANTIALIASED_QUALITY; // 启用抗锯齿
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    settextstyle(&font);
      settextcolor(RGB(0, 84, 153));
    setbkmode(TRANSPARENT);
    const TCHAR* prompt = _T("是否退出游戏？");
    int w = textwidth(prompt);
    outtextxy(1280/2 - w/2, 320, prompt);  // 720p适配

    // 获取当前鼠标位置
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);

    // 绘制按钮（带悬停效果）
    yesButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    noButton.drawWithHover(currentMousePos.x, currentMousePos.y);
}

StateNode* ExitState::handleEvent() {
    // 直接获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    if (yesButton.isClicked(pt.x, pt.y)) {
        return nullptr; // 结束循环退出程序
    } else if (noButton.isClicked(pt.x, pt.y)) {
        return &mainMenu; // 返回主菜单
    }
    return this;
}

// ContinueGameState - 继续游戏确认状态
void ContinueGameState::render() {
    // 全屏背景遮蔽
    setfillcolor(RGB(240, 245, 250)); // 浅蓝白色背景
    solidrectangle(0, 0, 1280, 720);
    
    // 在屏幕中央弹出白色对话框 (720p适配)
    setfillcolor(WHITE);
    setlinecolor(RGB(0, 84, 153));
    setlinestyle(PS_SOLID, 2);
    fillroundrect(390, 260, 890, 460, 10, 10);  // 圆角矩形

    // 启用抗锯齿字体渲染
    LOGFONT font;
    gettextstyle(&font);
    font.lfHeight = 28;
    font.lfWidth = 0;
    font.lfWeight = FW_NORMAL;
    font.lfQuality = ANTIALIASED_QUALITY; // 启用抗锯齿
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    settextstyle(&font);
      settextcolor(RGB(0, 84, 153));
    setbkmode(TRANSPARENT);
    const TCHAR* prompt = _T("是否继续上次的游戏？");
    int w = textwidth(prompt);
    outtextxy(1280/2 - w/2, 320, prompt);  // 720p适配

    // 获取当前鼠标位置
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);

    // 绘制按钮（带悬停效果）
    yesButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    noButton.drawWithHover(currentMousePos.x, currentMousePos.y);
}

StateNode* ContinueGameState::handleEvent() {
    // 直接获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
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

// RestartConfirmState - 重新开始确认状态
void RestartConfirmState::render() {
    // 全屏背景遮蔽
    setfillcolor(RGB(255, 245, 235)); // 浅橙白色背景
    solidrectangle(0, 0, 1280, 720);
    
    // 绘制橙色主题的对话框
    setfillcolor(RGB(255, 248, 240)); // 温暖的橙白色背景
    setlinecolor(RGB(255, 140, 0));   // 标准橙色边框
    setlinestyle(PS_SOLID, 3);
    fillroundrect(340, 200, 940, 500, 15, 15); // 圆角矩形对话框
    
    // 设置标题字体
    LOGFONT titleFont;
    gettextstyle(&titleFont);
    titleFont.lfHeight = 42;
    titleFont.lfWidth = 0;
    titleFont.lfWeight = FW_BOLD;
    titleFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(titleFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&titleFont);
    
    // 绘制标题
    settextcolor(RGB(255, 140, 0)); // 标准橙色
    setbkmode(TRANSPARENT);
    const TCHAR* title = _T("重新开始");
    int titleWidth = textwidth(title);
    outtextxy(640 - titleWidth/2, 240, title);
    
    // 设置内容字体
    LOGFONT contentFont;
    gettextstyle(&contentFont);
    contentFont.lfHeight = 28;
    contentFont.lfWidth = 0;
    contentFont.lfWeight = FW_NORMAL;
    contentFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(contentFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&contentFont);
    
    // 绘制确认问题
    settextcolor(RGB(102, 51, 0)); // 深橙棕色
    const TCHAR* prompt = _T("是否重新开始当前游戏？");
    int promptWidth = textwidth(prompt);
    outtextxy(640 - promptWidth/2, 310, prompt);
      // 绘制提示信息
    settextcolor(RGB(153, 102, 51)); // 中等橙棕色
    const TCHAR* info = _T("当前进度将会丢失");
    int infoWidth = textwidth(info);
    outtextxy(640 - infoWidth/2, 350, info);

    // 获取当前鼠标位置
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);

    // 创建橙色主题的按钮并绘制（带悬停效果）
    Button orangeYesButton(490, 390, 100, 40, _T("是"), RGB(255, 140, 0), RGB(204, 102, 0), WHITE);
    orangeYesButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    
    Button orangeNoButton(690, 390, 100, 40, _T("否"), RGB(255, 165, 0), RGB(255, 140, 0), WHITE);
    orangeNoButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    orangeNoButton.draw();
}

StateNode* RestartConfirmState::handleEvent() {
    // 直接获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    // 使用与渲染中相同的按钮坐标进行点击检测
    // 是按钮: (490, 390, 100, 40)
    if (pt.x >= 490 && pt.x <= 590 && pt.y >= 390 && pt.y <= 430) {
        gameState.restartCurrentMode(); // 重新开始当前模式
        return &gameState;
    } 
    // 否按钮: (690, 390, 100, 40)
    else if (pt.x >= 690 && pt.x <= 790 && pt.y >= 390 && pt.y <= 430) {
        return &gameState; // 返回游戏界面
    }
    return this;
}

// GameFailedState 实现
void GameFailedState::render() {
    // 全屏背景遮蔽
    setfillcolor(RGB(245, 242, 238)); // 浅暖白色背景
    solidrectangle(0, 0, 1280, 720);
    
    // 绘制温和橙红色的对话框
    setfillcolor(RGB(250, 240, 230)); // 温暖的米白色背景
    setlinecolor(RGB(220, 100, 80));  // 温和的橙红色边框
    setlinestyle(PS_SOLID, 3);
    fillroundrect(340, 200, 940, 500, 15, 15); // 圆角矩形对话框
    
    // 设置标题字体
    LOGFONT titleFont;
    gettextstyle(&titleFont);
    titleFont.lfHeight = 48;
    titleFont.lfWidth = 0;
    titleFont.lfWeight = FW_BOLD;
    titleFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(titleFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&titleFont);
    
    // 绘制标题
    settextcolor(RGB(220, 100, 80)); // 温和的橙红色
    setbkmode(TRANSPARENT);
    const TCHAR* title = _T("游戏结束");
    int titleWidth = textwidth(title);
    outtextxy(640 - titleWidth/2, 240, title);
    
    // 设置内容字体
    LOGFONT contentFont;
    gettextstyle(&contentFont);
    contentFont.lfHeight = 24;
    contentFont.lfWidth = 0;
    contentFont.lfWeight = FW_NORMAL;
    contentFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(contentFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&contentFont);
    
    // 绘制失败原因
    settextcolor(RGB(80, 80, 80)); // 深灰色
    const TCHAR* reason = _T("已无可移动的棋子！");
    int reasonWidth = textwidth(reason);
    outtextxy(640 - reasonWidth/2, 310, reason);
      // 绘制鼓励文字
    settextcolor(RGB(120, 120, 120)); // 浅灰色
    const TCHAR* encourage = _T("别灰心！孔明棋需要策略和耐心");
    int encourageWidth = textwidth(encourage);
    outtextxy(640 - encourageWidth/2, 350, encourage);
    
    // 获取当前鼠标位置
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    // 绘制按钮（带悬停效果）
    exitButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    continueButton.drawWithHover(currentMousePos.x, currentMousePos.y);
}

StateNode* GameFailedState::handleEvent() {
    // 获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (exitButton.isClicked(pt.x, pt.y)) {
        gameState.resetGame();
        return &chooseGame;
    } else if (continueButton.isClicked(pt.x, pt.y)) {
        // 直接悔棋，无需确认
        if (gameState.getBoard().canUndo()) {
            gameState.getBoard().undoMove();
        }
        return &gameState;
    }
    return this;
}

// GameWonState 实现
void GameWonState::render() {
    // 全屏背景遮蔽
    setfillcolor(RGB(250, 248, 240)); // 浅金白色背景
    solidrectangle(0, 0, 1280, 720);
    
    // 绘制金色胜利对话框
    setfillcolor(RGB(255, 248, 220)); // 淡金色背景
    setlinecolor(RGB(255, 215, 0));   // 金色边框
    setlinestyle(PS_SOLID, 3);
    fillroundrect(340, 200, 940, 500, 15, 15); // 圆角矩形对话框
    
    // 设置标题字体
    LOGFONT titleFont;
    gettextstyle(&titleFont);
    titleFont.lfHeight = 48;
    titleFont.lfWidth = 0;
    titleFont.lfWeight = FW_BOLD;
    titleFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(titleFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&titleFont);
    
    // 绘制标题
    settextcolor(RGB(255, 165, 0)); // 橙金色
    setbkmode(TRANSPARENT);
    const TCHAR* title = _T("恭喜胜利！");
    int titleWidth = textwidth(title);
    outtextxy(640 - titleWidth/2, 240, title);
    
    // 设置内容字体
    LOGFONT contentFont;
    gettextstyle(&contentFont);
    contentFont.lfHeight = 24;
    contentFont.lfWidth = 0;
    contentFont.lfWeight = FW_NORMAL;
    contentFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(contentFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&contentFont);
    
    // 绘制祝贺文字
    settextcolor(RGB(80, 80, 80)); // 深灰色
    const TCHAR* congratulation = _T("您成功完成了孔明棋！");
    int congratulationWidth = textwidth(congratulation);
    outtextxy(640 - congratulationWidth/2, 310, congratulation);
      // 绘制鼓励文字
    settextcolor(RGB(120, 120, 120)); // 浅灰色
    const TCHAR* encourage = _T("智慧与策略的完美结合");
    int encourageWidth = textwidth(encourage);
    outtextxy(640 - encourageWidth/2, 350, encourage);
    
    // 获取当前鼠标位置
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    // 绘制按钮（带悬停效果）
    nextGameButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    exitButton.drawWithHover(currentMousePos.x, currentMousePos.y);
}

StateNode* GameWonState::handleEvent() {
    // 获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (nextGameButton.isClicked(pt.x, pt.y)) {
        // 检查当前游戏模式，开始相应的下一局
        if (gameState.isEndgameMode()) {
            // 当前是残局模式，开始新的残局
            gameState.startEndgame();
        } else {
            // 当前是经典模式，重置并开始经典模式
            gameState.resetGame();
        }
        return &gameState;
    } else if (exitButton.isClicked(pt.x, pt.y)) {
        // 重置游戏并返回选择游戏界面
        gameState.resetGame();
        return &chooseGame;
    }
    return this;
}
