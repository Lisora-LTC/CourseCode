#include <vector>
#include <easyx.h>
#include <tchar.h>
#include "Solitare.h"
#include <unordered_map>

using namespace std;

MainMenuState mainMenu;
ChooseGameState chooseGame;
ExitState exitState;
GameState gameState;
ContinueGameState continueGameState;

// MainMenuState
void MainMenuState::render() {
    cleardevice(); // 清屏
    // 顶部深蓝条 (720p适配)
    setfillcolor(RGB(0, 84, 153));
    solidrectangle(0, 0, 1280, 100);
    // 页面标题
    pageTitle.draw();
    // 按钮
    startButton.draw();
    exitButton.draw();
}

StateNode* MainMenuState::handleEvent() {
    // 直接获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    if (startButton.isClicked(pt.x, pt.y)) {
        return &chooseGame;
    } else if (exitButton.isClicked(pt.x, pt.y)) {
        return &exitState;
    }
    return this;
}

// ChooseGameState
void ChooseGameState::render() {
    cleardevice(); // 清屏
    // 顶部深蓝条 (720p适配)
    setfillcolor(RGB(0, 84, 153));
    solidrectangle(0, 0, 1280, 100);
    // 页面标题
    pageTitle.draw();
    // 返回按钮
    returnButton.draw();
    startButton.draw();
}

StateNode* ChooseGameState::handleEvent() {
    // 直接获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);    if (returnButton.isClicked(pt.x, pt.y)) {
        return &mainMenu;
    } else if (startButton.isClicked(pt.x, pt.y)) {
        // 检查是否有进行中的游戏
        if (gameState.isGameStarted()) {
            return &continueGameState;  // 有进行中的游戏，弹出确认界面
        } else {
            return &gameState;  // 没有进行中的游戏，直接开始新游戏
        }
    }
    return this;
}

void GameState::render() {
    if (!boardInitialized) {
        BoardInit("English");  // 默认English
        boardInitialized = true;
    }
      // 获取鼠标位置并更新悬停状态
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    board.updateHover(pt.x, pt.y);
    
    cleardevice(); // 清屏
    // 顶部深蓝条 (720p适配)
    setfillcolor(RGB(0, 84, 153));
    solidrectangle(0, 0, 1280, 100);
    // 页面标题
    pageTitle.draw();
    // 返回按钮
    returnButton.draw();
    board.render();
}

StateNode* GameState::handleEvent() {
    // 直接获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);    
    if (returnButton.isClicked(pt.x, pt.y)) {
        // 重置游戏状态，下次进入时重新初始化
        boardInitialized = false;
        return &chooseGame; // 点击返回回到游戏选择界面
    }      // 处理棋盘点击
    if (board.handleClick(pt.x, pt.y)) {
        // 如果棋盘状态发生了改变（玩家移动了棋子），标记游戏已开始
        gameStarted = true;
        
        // 检查游戏胜负状态
        if (board.isGameWon()) {
            // 游戏胜利
            MessageBox(GetForegroundWindow(), _T("恭喜！您成功了！只剩下一个棋子！"), _T("游戏胜利"), MB_OK | MB_ICONINFORMATION);
            resetGame(); // 重置游戏
            return &chooseGame; // 返回选择界面
        } else if (board.isGameLost()) {
            // 游戏失败
            MessageBox(GetForegroundWindow(), _T("游戏失败！已无路可走！"), _T("游戏失败"), MB_OK | MB_ICONEXCLAMATION);
            resetGame(); // 重置游戏
            return &chooseGame; // 返回选择界面
        }
    }
    
    return this;
}

// GameState的BoardInit方法实现
void GameState::BoardInit(const std::string& boardName) {
    // 查找对应的坐标列表
    std::unordered_map<std::string, std::vector<Coord>>::const_iterator it = AllBoards.find(boardName);
    if (it == AllBoards.end()) {
        return; // 未找到，直接返回
    }
    const std::vector<Coord>& coords = it->second;
    
    // 清空旧格子
    board.clearBlocks();
    
    // 按照坐标列表添加格子并初始化棋子
    for (size_t i = 0; i < coords.size(); ++i) {
        int x = coords[i].first;
        int y = coords[i].second;
        board.addBlock(x, y);
        
        // English board中间位置(索引15，但我们没有添加那个位置到坐标列表)
        // 所有位置都有棋子，除了我们在坐标列表中跳过的中间位置
        board.setPieceAt(board.getBlockCount() - 1, true);
    }    // 为English board添加中间的空位置
    if (boardName == "English") {
        // 在索引15的位置添加空格子 (第4行第4个位置) - 720p适配
        board.addBlock(605, 375);
        board.setPieceAt(board.getBlockCount() - 1, false); // 中间位置开始为空
    }
}

// ExitState
void ExitState::render() {    // 在屏幕中央弹出白色对话框 (720p适配)
    setfillcolor(WHITE);
    solidrectangle(390, 260, 890, 460);  // 居中: (1280-500)/2=390, (720-200)/2=260
    setlinecolor(RGB(0, 84, 153));    rectangle(390, 260, 890, 460);
    
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

    // 绘制按钮
    yesButton.draw();
    noButton.draw();
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
    // 在屏幕中央弹出白色对话框 (720p适配)
    setfillcolor(WHITE);
    solidrectangle(390, 260, 890, 460);  // 居中: (1280-500)/2=390, (720-200)/2=260
    setlinecolor(RGB(0, 84, 153));    rectangle(390, 260, 890, 460);

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

    // 绘制按钮
    yesButton.draw();
    noButton.draw();
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
        return &gameState; // 开始新游戏
    }
    return this;
}

// GameState的新方法实现
void GameState::resetGame() {
    boardInitialized = false;
    gameStarted = false;
    board.clearBlocks();
}