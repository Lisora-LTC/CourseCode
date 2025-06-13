#include <vector>
#include <easyx.h>
#include <tchar.h>
#include "Solitare.h"
#include <unordered_map>
#include <cstdlib>
#include <ctime>

using namespace std; 

MainMenuState mainMenu;
ChooseGameState chooseGame;
HowToPlayState howToPlay; 
GameState gameState;

// MainMenuState
void MainMenuState::render() {
    cleardevice(); // 清屏
    // 顶部深蓝条 (720p适配)
    setfillcolor(RGB(0, 84, 153));
    solidrectangle(0, 0, 1280, 100);
    // 页面标题
    pageTitle.draw();
    
    // 获取当前鼠标位置
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    // 按钮（带悬停效果）
    startButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    howToPlayButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    exitButton.drawWithHover(currentMousePos.x, currentMousePos.y);
}

StateNode* MainMenuState::handleEvent() {
    // 直接获取鼠标点击位置并判断按钮
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
    solidrectangle(0, 0, 1280, 100);
    // 页面标题
    pageTitle.draw();
    
    // 获取当前鼠标位置
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    // 返回按钮（带悬停效果）
    returnButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    
    // 加载并显示英式棋盘图片
    static bool imageLoaded = false;
    static IMAGE boardImage;
    if (!imageLoaded) {
        loadimage(&boardImage, _T("EnglishBoard_resized.png"));
        imageLoaded = true;
    }
    
    // 获取图片尺寸
    int imgWidth = boardImage.getwidth();
    int imgHeight = boardImage.getheight();    // 检查图片是否有效（宽度和高度大于0）
    if (imgWidth > 0 && imgHeight > 0) {        // 在图片上方添加说明文字
        LOGFONT labelFont;
        gettextstyle(&labelFont);
        labelFont.lfHeight = 36; // 字体从28调大到36
        labelFont.lfWidth = 0;
        labelFont.lfWeight = FW_BOLD;
        labelFont.lfQuality = ANTIALIASED_QUALITY;
        _tcscpy_s(labelFont.lfFaceName, _T("微软雅黑"));
        settextstyle(&labelFont);
          settextcolor(RGB(0, 84, 153));
        setbkmode(TRANSPARENT);
        const TCHAR* label = _T("英式棋盘");
        int labelWidth = textwidth(label);
        outtextxy(640 - labelWidth/2, 150, label); // 标题位置调整到Y=150
          // 图片位置固定 - 在标题下方合适距离
        int imgX = (1280 - imgWidth) / 2;
        int imgY = 210; // 图片下移到Y=210，增加与标题的间距
        
        // 显示图片
        putimage(imgX, imgY, &boardImage);
    } else {
        // 图片加载失败，显示提示信息
        LOGFONT font;
        gettextstyle(&font);
        font.lfHeight = 24;
        font.lfWidth = 0;
        font.lfWeight = FW_NORMAL;
        font.lfQuality = ANTIALIASED_QUALITY;
        _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
        settextstyle(&font);
          settextcolor(RGB(0, 84, 153));
        setbkmode(TRANSPARENT);
        const TCHAR* errorText = _T("英式孔明棋棋盘布局");
        int textWidth = textwidth(errorText);
        outtextxy(640 - textWidth/2, 380, errorText); // 相应调整错误提示位置
    }
    // 绘制开始游戏按钮（下移到580位置，与图片保持距离）
    startButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    // 绘制残局模式按钮
    endgameButton.drawWithHover(currentMousePos.x, currentMousePos.y);
}

StateNode* ChooseGameState::handleEvent() {
    // 直接获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);    
    if (returnButton.isClicked(pt.x, pt.y)) {
        return &mainMenu;    } else if (startButton.isClicked(pt.x, pt.y)) {
        // 检查是否有进行中的游戏
        pendingEndgame = false;  // 标记为普通游戏模式
        if (gameState.isGameStarted()) {
            return &continueGameState;  // 有进行中的游戏，弹出确认界面
        } else {
            return &gameState;  // 没有进行中的游戏，直接开始新游戏
        }
    } else if (endgameButton.isClicked(pt.x, pt.y)) {
        // 残局模式也需要检查是否有进行中的游戏
        pendingEndgame = true;  // 标记为残局模式
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
    solidrectangle(0, 0, 1280, 100);    // 页面标题
    pageTitle.draw();
    
    // 获取当前鼠标位置
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    // 返回按钮（带悬停效果）
    returnButton.drawWithHover(currentMousePos.x, currentMousePos.y);    // 悔棋按钮 - 永远显示，但根据栈状态改变颜色和可按状态
    if (board.canUndo()) {
        // 有历史记录时显示蓝色，可按
        Button enabledUndoButton(1150, 340, 100, 40, _T("悔棋"), 
                                 RGB(0, 120, 215), RGB(0, 84, 153), WHITE, true);
        enabledUndoButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    } else {
        // 没有历史记录时显示灰色，不可按
        Button disabledUndoButton(1150, 340, 100, 40, _T("悔棋"), 
                                  RGB(128, 128, 128), RGB(96, 96, 96), RGB(192, 192, 192), false);
        disabledUndoButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    }
    
    // 重新开始按钮 - 永远显示为橙色
    restartButton.drawWithHover(currentMousePos.x, currentMousePos.y);
    
    // 渲染图例
    renderLegend();
    // 渲染棋盘
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
    }
      // 处理悔棋按钮点击 - 检查按钮区域而不是特定按钮对象
    if (pt.x >= 1150 && pt.x <= 1250 && pt.y >= 340 && pt.y <= 380) {
        if (board.canUndo()) {  // 只在有历史记录时执行悔棋
            board.undoMove();  // 执行悔棋
        }
        return this;  // 保持在游戏状态
    }
    
    // 处理重新开始按钮点击
    if (restartButton.isClicked(pt.x, pt.y)) {
        return &restartConfirmState;  // 跳转到重新开始确认界面
    }

// 处理棋盘点击
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

// 新增：重新开始当前模式
void GameState::restartCurrentMode() {
    if (endgameMode) {
        // 当前是残局模式，生成新的残局
        startEndgame();
    } else {
        // 当前是经典模式，重置游戏
        resetGame();
    }
}

// GameState 图例渲染方法实现
// GameState 图例渲染方法实现

// GameState的新方法实现
void GameState::resetGame() {
    boardInitialized = false;
    gameStarted = false;
    endgameMode = false;  // 重置残局模式标志
    board.clearBlocks();
    board.clearHistory();  // 🔥 清空悔棋历史
}

// 新增：残局模式初始化，随机多步生成残局
void GameState::startEndgame() {
    // 构建空棋盘
    board.clearBlocks();
    // 添加所有格子并置空
    for (const auto& coord : AllBoards.at("English")) {
        board.addBlock(coord.first, coord.second);
        board.setPieceAt(board.getBlockCount() - 1, false);
    }
    // 添加中心空格
    board.addBlock(605, 375);
    board.setPieceAt(board.getBlockCount() - 1, false);
    
    // 随机选择最后一颗棋子位置
    std::srand((unsigned)std::time(nullptr));
    int count = board.getBlockCount();
    int finalIdx = std::rand() % count;
    board.setPieceAt(finalIdx, true);
    
    // 随机多步反向移动，生成残局
    int steps = std::rand() % 10 + 5; // 5到14步
    for (int i = 0; i < steps; ++i) {
        auto revs = board.getReverseMoves();
        if (revs.empty()) break;
        int idx = std::rand() % revs.size();
        board.applyReverseMove(revs[idx]);
    }
    
    board.clearHistory();  // 清空反向移动历史
    boardInitialized = true;
    gameStarted = true;    endgameMode = true;
}

// GameState 图例渲染方法实现
void GameState::renderLegend() const {
    // 图例标题
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
    
    // 设置说明文字字体
    LOGFONT textFont;
    gettextstyle(&textFont);
    textFont.lfHeight = 18;
    textFont.lfWidth = 0;
    textFont.lfWeight = FW_NORMAL;
    textFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(textFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&textFont);
    settextcolor(RGB(80, 80, 80));
    
    // 普通棋子图例 - 增加间距
    renderLegendPiece(80, 180, 20, RGB(70, 130, 220), RGB(20, 50, 120));
    outtextxy(120, 170, _T("普通棋子"));
    
    // 选中棋子图例 - 增加间距
    renderLegendPiece(80, 250, 20, RGB(50, 205, 50), RGB(34, 139, 34));
    // 选中光环效果
    setlinecolor(RGB(50, 205, 50));
    setlinestyle(PS_SOLID, 2);
    circle(80, 250, 28);
    outtextxy(120, 240, _T("选中棋子"));
    
    // 可走位置图例 - 增加间距
    renderLegendMovable(80, 320, 20);
    outtextxy(120, 310, _T("可走位置"));
}

void GameState::renderLegendPiece(int x, int y, int radius, COLORREF fillColor, COLORREF borderColor, bool hasHighlight) const {
    // 绘制格子底座（白色圆形凹槽，蓝色边框）
    setfillcolor(RGB(250, 252, 255));  // 纯净白色
    setlinecolor(RGB(70, 130, 220));   // 清爽蓝色边框
    setlinestyle(PS_SOLID, 1);
    solidcircle(x, y, radius + 4);
    circle(x, y, radius + 4);
    
    // 内圈阴影效果（浅蓝色）
    setfillcolor(RGB(240, 245, 255));  // 极浅蓝色
    solidcircle(x, y, radius + 2);
    
    // 绘制棋子
    setfillcolor(fillColor);
    solidcircle(x, y, radius);
    
    // 高光效果（白色）
    if (hasHighlight) {
        setfillcolor(RGB(255, 255, 255));
        solidcircle(x - 6, y - 6, radius / 3);
    }
    
    // 深色边框增强对比度
    setlinecolor(borderColor);
    setlinestyle(PS_SOLID, 2);
    circle(x, y, radius);
}

void GameState::renderLegendMovable(int x, int y, int radius) const {
    // 将整个格子底座改为橙红色背景
    setfillcolor(RGB(255, 140, 100));  // 浅橙红色背景
    solidcircle(x, y, radius + 4);
    
    // 橙红色边框，与棋盘大小相等
    setlinecolor(RGB(255, 69, 0));     // 鲜艳橙红色边框
    setlinestyle(PS_SOLID, 3);         // 加粗边框使其更明显
    circle(x, y, radius + 4);
    
    // 内圈橙红色阴影效果
    setfillcolor(RGB(255, 180, 150));  // 更浅的橙红色
    solidcircle(x, y, radius + 2);
    
    // 中心橙红色圆形指示器
    setfillcolor(RGB(255, 69, 0));     // 鲜艳橙红色
    solidcircle(x, y, radius * 2 / 3);
    
    // 白色高光效果
    setfillcolor(RGB(255, 255, 255));
    solidcircle(x - 4, y - 4, radius / 4);
    
    // 深橙红色边框增强对比度
    setlinecolor(RGB(200, 50, 0));
    setlinestyle(PS_SOLID, 1);
    circle(x, y, radius * 2 / 3);
}

// HowToPlayState 实现 - 玩法介绍页面
void HowToPlayState::render() {
    cleardevice(); // 清屏
    // 顶部深蓝条 (720p适配)
    setfillcolor(RGB(0, 84, 153));
    solidrectangle(0, 0, 1280, 100);
    // 页面标题
    pageTitle.draw();
    
    // 获取当前鼠标位置
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);
    ScreenToClient(GetForegroundWindow(), &currentMousePos);
    
    // 返回按钮（带悬停效果）
    returnButton.drawWithHover(currentMousePos.x, currentMousePos.y);
      // 设置内容字体 - 适中大小加粗
    LOGFONT contentFont;
    gettextstyle(&contentFont);
    contentFont.lfHeight = 26;  // 从32减少到26
    contentFont.lfWidth = 0;
    contentFont.lfWeight = FW_BOLD;
    contentFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(contentFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&contentFont);
    
    settextcolor(RGB(0, 84, 153));
    setbkmode(TRANSPARENT);
    
    // 紧凑的玩法介绍内容
    int startY = 140;  // 从180减少到140
    int lineHeight = 40;  // 从55减少到40
    
    outtextxy(100, startY, _T("游戏目标：通过跳跃吃子，最终只剩一枚棋子"));
    
    outtextxy(100, startY + lineHeight * 2, _T("基本规则："));
    outtextxy(150, startY + lineHeight * 3, _T("• 跳过相邻棋子到空位    • 被跳过的棋子消失"));
    outtextxy(150, startY + lineHeight * 4, _T("• 只能上下左右移动      • 选中棋子显示绿色"));
      outtextxy(100, startY + lineHeight * 6, _T("操作方法："));
    outtextxy(150, startY + lineHeight * 7, _T("• 点击棋子选中          • 点击橙红色位置移动"));
    outtextxy(150, startY + lineHeight * 8, _T("• 支持悔棋功能          • 右侧按钮可撤销"));
      // 右侧展示Lisora.png图片
    IMAGE* lisora_img = new IMAGE;
    loadimage(lisora_img, _T("Lisora.png"));
    
    // 计算图片位置 - 右侧居中
    int imgWidth = lisora_img->getwidth();
    int imgHeight = lisora_img->getheight();
    int imgX = 900;  // 右侧位置
    int imgY = startY + lineHeight * 2;  // 从规则部分开始的高度
    
    // 绘制图片
    putimage(imgX, imgY, lisora_img);
    
    // 在图片下方显示加粗的姓名
    LOGFONT nameFont;
    gettextstyle(&nameFont);
    nameFont.lfHeight = 32;  // 大字体
    nameFont.lfWidth = 0;
    nameFont.lfWeight = FW_BOLD;  // 加粗
    nameFont.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(nameFont.lfFaceName, _T("微软雅黑"));
    settextstyle(&nameFont);
    
    settextcolor(RGB(0, 84, 153));
    const TCHAR* name = _T("Lisora");
    int nameWidth = textwidth(name);
    int nameX = imgX + (imgWidth - nameWidth) / 2;  // 居中对齐图片
    int nameY = imgY + imgHeight + 20;  // 图片下方20像素
    outtextxy(nameX, nameY, name);
    
    delete lisora_img;
}

StateNode* HowToPlayState::handleEvent() {
    // 获取鼠标点击位置并判断按钮
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetForegroundWindow(), &pt);
    
    if (returnButton.isClicked(pt.x, pt.y)) {
        return &mainMenu; // 返回主菜单
    }
    return this;
}