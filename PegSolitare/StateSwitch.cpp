#include <vector>
#include <easyx.h>
#include <tchar.h>
#include <windows.h>  // 为 Sleep
#include "Solitare.h"
#include <unordered_map>
#include <cstdlib>
#include <ctime>

using namespace std; 

MainMenuState mainMenu;
ChooseGameState chooseGame;
HowToPlayState howToPlay; 
GameState gameState;

extern MoveRecord searchBestMove(const Chessboard& board);
static int hintFromIndex = -1;
static int hintToIndex = -1;
static bool hintSearching = false;

// MainMenuState
void MainMenuState::render() {
    cleardevice();
    setfillcolor(RGB(0, 84, 153));
    solidrectangle(0, 0, 1280, 100);
    pageTitle.draw();    
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    startButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    howToPlayButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    exitButton.drawWithHover(currentMousePos.x, currentMousePos.y);
}

StateNode* MainMenuState::handleEvent() {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    if (startButton.isClicked(pt.x, pt.y)) {
        return &chooseGame;
    } else if (howToPlayButton.isClicked(pt.x, pt.y)) {
        return &howToPlay;
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
    solidrectangle(0, 0, 1280, 100);    pageTitle.draw();
    
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    returnButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    renderImage(_T("EnglishBoard_resized.png"), _T("英式棋盘"));
    startButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    endgameButton.drawWithHover(currentMousePos.x, currentMousePos.y);
}

StateNode* ChooseGameState::handleEvent() {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (returnButton.isClicked(pt.x, pt.y)) {
        return &mainMenu;
    } else if (startButton.isClicked(pt.x, pt.y)) {
        pendingEndgame = false;
        if (gameState.isGameStarted()) {
            return &continueGameState;
        } else {
            return &gameState;
        }
    } else if (endgameButton.isClicked(pt.x, pt.y)) {
        pendingEndgame = true;
        if (gameState.isGameStarted()) {
            return &continueGameState;  // 有进行中的游戏，弹出确认界面
        } else {
            gameState.startEndgame();
            return &gameState;
        }
    }
    return this;
}

void GameState::render() {
    if (!boardInitialized) {
        BoardInit("English");
        boardInitialized = true;
    }
    
    POINT mousePos;
    GetCursorPos(&mousePos);
    ScreenToClient(GetForegroundWindow(), &mousePos);
    board.updateHover(mousePos.x, mousePos.y);
    
    cleardevice();
    setfillcolor(RGB(0, 84, 153));
    solidrectangle(0, 0, 1280, 100);
    
    pageTitle.draw();
    renderStatusText();
    returnButton.drawWithHover(mousePos.x, mousePos.y);
    
    // 悔棋按钮状态切换
    if (board.canUndo()) {
        Button enabledUndoButton(1150, 350, 100, 40, _T("悔棋"), 
                                 RGB(0, 120, 215), RGB(0, 84, 153), WHITE, true);
        enabledUndoButton.drawWithHover(mousePos.x, mousePos.y);
    } else {
        Button disabledUndoButton(1150, 350, 100, 40, _T("悔棋"), 
                                  RGB(128, 128, 128), RGB(96, 96, 96), RGB(192, 192, 192), false);
        disabledUndoButton.drawWithHover(mousePos.x, mousePos.y);
    }
    
    restartButton.drawWithHover(mousePos.x, mousePos.y);
    hintButton.drawWithHover(mousePos.x, mousePos.y);
    renderLegend();
    board.render();
    
    // 提示高亮框绘制
    if (hintFromIndex >= 0 && hintToIndex >= 0) {
        int fx = EnglishCoords[hintFromIndex].first;
        int fy = EnglishCoords[hintFromIndex].second;
        setlinecolor(RGB(255,0,0)); setlinestyle(PS_SOLID,3);
        rectangle(fx, fy, fx + 70, fy + 70);
        int tx = EnglishCoords[hintToIndex].first;
        int ty = EnglishCoords[hintToIndex].second;
        setlinecolor(RGB(0,255,0)); setlinestyle(PS_SOLID,3);
        rectangle(tx, ty, tx + 70, ty + 70);
    }
}

StateNode* GameState::handleEvent() {
    hintFromIndex = -1;
    hintToIndex = -1;
    
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (returnButton.isClicked(pt.x, pt.y)) {
        return &chooseGame;
    }
    
    // 悔棋按钮处理
    if (undoButton.isClicked(pt.x,pt.y)) {
        if (board.canUndo()) {
            board.undoMove();
            setStatusText(_T(""), RGB(255, 255, 255));
        }
        return this;
    }
    
    if (restartButton.isClicked(pt.x, pt.y)) {
        return &restartConfirmState;
    }

    // 棋盘点击处理
    if (board.handleClick(pt.x, pt.y)) {
        gameStarted = true;
        setStatusText(_T(""), RGB(255, 255, 255));
        
        if (board.isGameWon()) {
            return &gameWonState;
        } else if (board.isGameLost()) {
            return &gameFailedState;
        }
    }
    
    // 提示按钮处理
    if (hintButton.isClicked(pt.x, pt.y)) {
        board.clearAllTargets();
        board.clearAllHintFrom();
        hintFromIndex = -1;
        hintToIndex = -1;
        
        hintSearching = true;
        setStatusText(_T("正在搜索..."), RGB(255, 255, 0));
        this->render();
        FlushBatchDraw();
        Sleep(200);
        
        MoveRecord rec = searchBestMove(board);
        hintSearching = false;
        
        if (rec.fromIndex >= 0 && rec.toIndex >= 0) {
            board.setHintFromAt(rec.fromIndex, true);
            board.setTargetAt(rec.toIndex, true);
            setStatusText(_T("搜索成功"), RGB(0, 255, 0));
        } else {
            setStatusText(_T("搜索失败"), RGB(255, 0, 0));
        }
        
        return &gameState;
    }
    
    return this;
}

// 棋盘初始化
void GameState::BoardInit(const std::string& boardName) {
    std::unordered_map<std::string, std::vector<Coord>>::const_iterator it = AllBoards.find(boardName);
    if (it == AllBoards.end()) {
        return;
    }
    const std::vector<Coord>& coords = it->second;
    
    board.clearBlocks();
    
    for (size_t i = 0; i < coords.size(); ++i) {
        int x = coords[i].first;
        int y = coords[i].second;
        board.addBlock(x, y);
        
        if (boardName == "English" && x == 605 && y == 375) {
            board.setPieceAt(board.getBlockCount() - 1, false);
        } else {
            board.setPieceAt(board.getBlockCount() - 1, true);
        }
    }
}

// 重新开始当前模式
void GameState::restartCurrentMode() {
    if (endgameMode) {
        startEndgame();
    } else {
        resetGame();
    }
}

// 游戏重置
void GameState::resetGame() {
    boardInitialized = false;
    gameStarted = false;
    endgameMode = false;
    board.clearAllTargets();
    board.clearAllHintFrom();
    board.clearBlocks();
    board.clearHistory();
    setStatusText(_T(""), RGB(255, 255, 255));
}

// 残局模式初始化
void GameState::startEndgame() {
    board.clearBlocks();
    
    for (const auto& coord : AllBoards.at("English")) {
        board.addBlock(coord.first, coord.second);
        board.setPieceAt(board.getBlockCount() - 1, false);
    }
    
    std::srand((unsigned)std::time(nullptr));
    int count = board.getBlockCount();
    int finalIdx = std::rand() % count;
    board.setPieceAt(finalIdx, true);
    
    int steps = std::rand() % 10 + 5;
    for (int i = 0; i < steps; ++i) {
        auto revs = board.getReverseMoves();
        if (revs.empty()) break;
        int idx = std::rand() % revs.size();
        board.applyReverseMove(revs[idx]);
    }
    
    board.clearHistory();
    boardInitialized = true;
    gameStarted = true;
    endgameMode = true;
}

// 图例渲染
void GameState::renderLegend() const {
    LOGFONT titleFont;
    gettextstyle(&titleFont);
    titleFont.lfHeight = 24;
    titleFont.lfWidth = 0;
    titleFont.lfWeight = FW_BOLD;
    titleFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(titleFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&titleFont);
    
    settextcolor(RGB(70, 130, 220));
    setbkmode(TRANSPARENT);
    const TCHAR* legendTitle = _T("图例说明");
    outtextxy(50, 120, legendTitle);
    
    LOGFONT textFont;
    gettextstyle(&textFont);
    textFont.lfHeight = 18;
    textFont.lfWidth = 0;
    textFont.lfWeight = FW_NORMAL;
    textFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(textFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&textFont);
    settextcolor(RGB(80, 80, 80));
    
    renderLegendPiece(80, 180, 20, RGB(70, 130, 220), RGB(20, 50, 120));
    outtextxy(120, 170, _T("普通棋子"));
    
    renderLegendPiece(80, 250, 20, RGB(50, 205, 50), RGB(34, 139, 34));
    setlinecolor(RGB(50, 205, 50));
    setlinestyle(PS_SOLID, 2);
    circle(80, 250, 28);
    outtextxy(120, 240, _T("选中棋子"));
    
    renderLegendMovable(80, 320, 20);
    outtextxy(120, 310, _T("可走位置"));
    
    renderLegendPiece(80, 390, 20, RGB(70, 130, 220), RGB(20, 50, 120));
    setlinecolor(RGB(255, 215, 0));
    setlinestyle(PS_SOLID, 2);
    circle(80, 390, 28);
    outtextxy(120, 380, _T("提示起始"));
    
    renderLegendTarget(80, 460, 20);
    outtextxy(120, 450, _T("提示目标"));
}

void GameState::renderLegendPiece(int x, int y, int radius, COLORREF fillColor, COLORREF borderColor, bool hasHighlight) const {
    setfillcolor(RGB(250, 252, 255));
    setlinecolor(RGB(70, 130, 220));
    setlinestyle(PS_SOLID, 1);
    solidcircle(x, y, radius + 4);
    circle(x, y, radius + 4);
    
    setfillcolor(RGB(240, 245, 255));
    solidcircle(x, y, radius + 2);
    
    setfillcolor(fillColor);
    solidcircle(x, y, radius);
    
    if (hasHighlight) {
        setfillcolor(RGB(255, 255, 255));
        solidcircle(x - 6, y - 6, radius / 3);
    }
    
    setlinecolor(borderColor);
    setlinestyle(PS_SOLID, 2);
    circle(x, y, radius);
}

void GameState::renderLegendMovable(int x, int y, int radius) const {
    setfillcolor(RGB(255, 140, 100));
    solidcircle(x, y, radius + 4);
    
    setlinecolor(RGB(255, 69, 0));
    setlinestyle(PS_SOLID, 3);
    circle(x, y, radius + 4);
    
    setfillcolor(RGB(255, 180, 150));
    solidcircle(x, y, radius + 2);
    
    setfillcolor(RGB(255, 69, 0));
    solidcircle(x, y, radius * 2 / 3);
    
    setfillcolor(RGB(255, 255, 255));
    solidcircle(x - 4, y - 4, radius / 4);
    
    setlinecolor(RGB(200, 50, 0));
    setlinestyle(PS_SOLID, 1);
    circle(x, y, radius * 2 / 3);
}

void GameState::renderLegendTarget(int x, int y, int radius) const {
    setfillcolor(RGB(255, 215, 100));
    solidcircle(x, y, radius + 4);
    
    setlinecolor(RGB(255, 215, 0));
    setlinestyle(PS_SOLID, 3);
    circle(x, y, radius + 4);
    
    setfillcolor(RGB(255, 235, 150));
    solidcircle(x, y, radius + 2);
    
    setfillcolor(RGB(255, 215, 0));
    solidcircle(x, y, radius * 2 / 3);
    
    setfillcolor(RGB(255, 255, 255));
    solidcircle(x - 4, y - 4, radius / 3);
    
    setlinecolor(RGB(184, 134, 11));
    setlinestyle(PS_SOLID, 1);
    circle(x, y, radius * 2 / 3);
    
    setlinecolor(RGB(255, 215, 0));
    setlinestyle(PS_SOLID, 2);
    circle(x, y, radius + 8);
}

// 玩法介绍页面
void HowToPlayState::render() {
    cleardevice();
    setfillcolor(RGB(0, 84, 153));
    solidrectangle(0, 0, 1280, 100);
    pageTitle.draw();
    
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    returnButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    
    LOGFONT contentFont;
    gettextstyle(&contentFont);
    contentFont.lfHeight = 26;
    contentFont.lfWidth = 0;
    contentFont.lfWeight = FW_BOLD;
    contentFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(contentFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&contentFont);
    
    settextcolor(RGB(0, 84, 153));
    setbkmode(TRANSPARENT);
    
    int startY = 140;
    int lineHeight = 40;
    
    outtextxy(100, startY, _T("游戏目标：通过跳跃吃子，最终只剩一枚棋子"));
    
    outtextxy(100, startY + lineHeight * 2, _T("基本规则："));
    outtextxy(150, startY + lineHeight * 3, _T("• 跳过相邻棋子到空位    • 被跳过的棋子消失"));
    outtextxy(150, startY + lineHeight * 4, _T("• 只能上下左右移动      • 选中棋子显示绿色"));
    
    outtextxy(100, startY + lineHeight * 6, _T("操作方法："));
    outtextxy(150, startY + lineHeight * 7, _T("• 点击棋子选中          • 点击橙红色位置移动"));
    outtextxy(150, startY + lineHeight * 8, _T("• 支持悔棋功能          • 右侧按钮可撤销"));
    
    // 右侧图片展示
    static IMAGE lisora_img;
    static bool imageLoaded = false;
    
    if (!imageLoaded) {
        loadimage(&lisora_img, _T("Lisora.png"));
        imageLoaded = true;
    }
    
    int imgWidth = lisora_img.getwidth();
    int imgHeight = lisora_img.getheight();
    int imgX = 900;
    int imgY = startY + lineHeight * 2;
    
    putimage(imgX, imgY, &lisora_img);
    
    LOGFONT nameFont;
    gettextstyle(&nameFont);
    nameFont.lfHeight = 32;
    nameFont.lfWidth = 0;
    nameFont.lfWeight = FW_BOLD;
    nameFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(nameFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&nameFont);
    
    settextcolor(RGB(0, 84, 153));
    const TCHAR* name = _T("作者：Lisora");
    int nameWidth = textwidth(name);
    int nameX = imgX + (imgWidth - nameWidth) / 2;
    int nameY = imgY + imgHeight + 20;
    outtextxy(nameX, nameY, name);
}

StateNode* HowToPlayState::handleEvent() {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (returnButton.isClicked(pt.x, pt.y)) {
        return &mainMenu;
    }
    return this;
}

// 状态文本处理
void GameState::setStatusText(const TCHAR* text, COLORREF color) {
    statusText = text;
    statusTextColor = color;
}

void GameState::renderStatusText() const {
    if (statusText && _tcslen(statusText) > 0) {
        settextcolor(statusTextColor);
        
        LOGFONT font;
        gettextstyle(&font);
        font.lfHeight = 60;
        font.lfWidth = 0;
        font.lfWeight = FW_NORMAL;
        font.lfQuality = ANTIALIASED_QUALITY;
        _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
        settextstyle(&font);
        
        int textWidth = textwidth(statusText);
        int x = 1280 - textWidth - 20;
        int y = 25;
        
        setbkmode(TRANSPARENT);
        outtextxy(x, y, statusText);
    }
}