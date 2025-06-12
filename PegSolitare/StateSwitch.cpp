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
GameFailedState gameFailedState;
GameWonState gameWonState;

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
    ScreenToClient(GetForegroundWindow(), &pt);    
    if (returnButton.isClicked(pt.x, pt.y)) {
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
        // 不重置游戏状态，保持棋盘当前状态
        // boardInitialized = false; // 🔧 移除这行，保持棋盘状态
        return &chooseGame; // 点击返回回到游戏选择界面
    }// 处理棋盘点击
    if (board.handleClick(pt.x, pt.y)) {
        // 如果棋盘状态发生了改变（玩家移动了棋子），标记游戏已开始
        gameStarted = true;
          // 检查游戏胜负状态
        if (board.isGameWon()) {
            // 游戏胜利 - 切换到胜利状态界面
            return &gameWonState;
        } else if (board.isGameLost()) {
            // 游戏失败 - 切换到失败状态界面
            return &gameFailedState;
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
void ExitState::render() {
    // 不清屏，直接在当前画面上绘制对话框
    
    // 创建模糊效果：在背景上绘制点状图案来模拟模糊
    setfillcolor(RGB(200, 210, 220)); // 浅灰蓝色点
    for (int i = 0; i < 1280; i += 4) {
        for (int j = 0; j < 720; j += 4) {
            if ((i + j) % 8 == 0) { // 创建棋盘格效果
                solidcircle(i, j, 1);
            }
        }
    }
    
    // 再添加一层半透明效果（用网格模拟）
    setlinecolor(RGB(240, 248, 255)); // 与背景色相近的颜色
    for (int i = 0; i < 1280; i += 2) {
        line(i, 0, i, 720);
    }
    for (int j = 0; j < 720; j += 2) {
        line(0, j, 1280, j);
    }
    
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
    // 不清屏，直接在当前画面上绘制对话框
    
    // 创建模糊效果：在背景上绘制点状图案来模拟模糊
    setfillcolor(RGB(200, 210, 220)); // 浅灰蓝色点
    for (int i = 0; i < 1280; i += 4) {
        for (int j = 0; j < 720; j += 4) {
            if ((i + j) % 8 == 0) { // 创建棋盘格效果
                solidcircle(i, j, 1);
            }
        }
    }
    
    // 再添加一层半透明效果（用网格模拟）
    setlinecolor(RGB(240, 248, 255)); // 与背景色相近的颜色
    for (int i = 0; i < 1280; i += 2) {
        line(i, 0, i, 720);
    }
    for (int j = 0; j < 720; j += 2) {
        line(0, j, 1280, j);
    }
    
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

// GameFailedState 实现
void GameFailedState::render() {
    // 不清屏，直接在当前画面上绘制对话框
    
    // 创建模糊效果：在背景上绘制点状图案来模拟模糊
    setfillcolor(RGB(200, 210, 220)); // 浅灰蓝色点
    for (int i = 0; i < 1280; i += 4) {
        for (int j = 0; j < 720; j += 4) {
            if ((i + j) % 8 == 0) { // 创建棋盘格效果
                solidcircle(i, j, 1);
            }
        }
    }
    
    // 再添加一层半透明效果（用网格模拟）
    setlinecolor(RGB(240, 248, 255)); // 与背景色相近的颜色
    for (int i = 0; i < 1280; i += 2) {
        line(i, 0, i, 720);
    }
    for (int j = 0; j < 720; j += 2) {
        line(0, j, 1280, j);
    }
    
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
    
    // 绘制按钮
    exitButton.draw();
    continueButton.draw();
}

StateNode* GameFailedState::handleEvent() {
    // 获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (exitButton.isClicked(pt.x, pt.y)) {
        // 重置游戏并返回选择游戏界面
        gameState.resetGame();
        return &chooseGame;
    } else if (continueButton.isClicked(pt.x, pt.y)) {
        // 暂时只返回游戏界面，后续可添加悔棋功能
        // TODO: 实现悔棋功能
        return &gameState;
    }
    return this;
}

// GameWonState 实现
void GameWonState::render() {
    // 不清屏，直接在当前画面上绘制对话框
    
    // 创建模糊效果：在背景上绘制点状图案来模拟模糊
    setfillcolor(RGB(200, 220, 240)); // 浅蓝色点
    for (int i = 0; i < 1280; i += 4) {
        for (int j = 0; j < 720; j += 4) {
            if ((i + j) % 8 == 0) { // 创建棋盘格效果
                solidcircle(i, j, 1);
            }
        }
    }
    
    // 再添加一层半透明效果（用网格模拟）
    setlinecolor(RGB(240, 248, 255)); // 与背景色相近的颜色
    for (int i = 0; i < 1280; i += 2) {
        line(i, 0, i, 720);
    }
    for (int j = 0; j < 720; j += 2) {
        line(0, j, 1280, j);
    }
    
    // 绘制蓝色主题的对话框
    setfillcolor(RGB(240, 248, 255)); // 浅蓝色背景
    setlinecolor(RGB(13, 110, 253));  // Bootstrap蓝色边框
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
    settextcolor(RGB(13, 110, 253)); // Bootstrap蓝色
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
    
    // 绘制胜利信息
    settextcolor(RGB(80, 80, 80)); // 深灰色
    const TCHAR* reason = _T("成功完成！只剩下一个棋子！");
    int reasonWidth = textwidth(reason);
    outtextxy(640 - reasonWidth/2, 310, reason);
    
    // 绘制鼓励文字
    settextcolor(RGB(120, 120, 120)); // 浅灰色
    const TCHAR* encourage = _T("太棒了！您掌握了孔明棋的精髓");
    int encourageWidth = textwidth(encourage);
    outtextxy(640 - encourageWidth/2, 350, encourage);
    
    // 绘制按钮
    nextGameButton.draw();
    exitButton.draw();
}

StateNode* GameWonState::handleEvent() {
    // 获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (nextGameButton.isClicked(pt.x, pt.y)) {
        // 重置游戏并开始新游戏
        gameState.resetGame();
        return &gameState;
    } else if (exitButton.isClicked(pt.x, pt.y)) {
        // 重置游戏并返回选择游戏界面
        gameState.resetGame();
        return &chooseGame;
    }
    return this;
}