#include "Solitare.h"
#include <cmath>
#include <algorithm>

// Title 类方法实现
// 在屏幕中央绘制标题文字
void Title::draw() const {
    LOGFONT font;
    gettextstyle(&font);
    font.lfHeight = fontSize;
    font.lfWidth = 0;
    font.lfWeight = FW_NORMAL;
    font.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    settextstyle(&font);
    
    int w = textwidth(text);
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    outtextxy(1280/2 - w/2, y, text);
}

// SingleBlock 类方法实现
// 构造函数，初始化棋盘格子的基本属性
SingleBlock::SingleBlock(int _x, int _y, int _w, int _h)
    : x(_x), y(_y), width(_w), height(_h), hasPiece(false), isHovered(false), isSelected(false), isMovable(false), isTarget(false), isHintFrom(false) {}
// 检查格子是否有棋子
bool SingleBlock::containsPiece() const { return hasPiece; }
// 设置格子是否有棋子
void SingleBlock::setPiece(bool val) { hasPiece = val; }
// 获取格子悬停状态
bool SingleBlock::getHovered() const { return isHovered; }
// 设置格子悬停状态
void SingleBlock::setHovered(bool hovered) { isHovered = hovered; }
// 获取格子选中状态
bool SingleBlock::getSelected() const { return isSelected; }
// 设置格子选中状态
void SingleBlock::setSelected(bool selected) { isSelected = selected; }
// 获取格子可移动状态
bool SingleBlock::getMovable() const { return isMovable; }
// 设置格子可移动状态
void SingleBlock::setMovable(bool movable) { isMovable = movable; }
// 获取格子目标状态
bool SingleBlock::getTarget() const { return isTarget; }
// 设置格子目标状态
void SingleBlock::setTarget(bool target) { isTarget = target; }
// 获取格子提示起始状态
bool SingleBlock::getHintFrom() const { return isHintFrom; }
// 设置格子提示起始状态
void SingleBlock::setHintFrom(bool hintFrom) { isHintFrom = hintFrom; }
// 检查指定坐标是否在格子内
bool SingleBlock::containsPoint(int px, int py) const { return px >= x && px <= x + width && py >= y && py <= y + height; }
// 获取格子X坐标
int SingleBlock::getX() const { return x; }
// 获取格子Y坐标
int SingleBlock::getY() const { return y; }
// 获取格子宽度
int SingleBlock::getWidth() const { return width; }
// 获取格子高度
int SingleBlock::getHeight() const { return height; }

// Chessboard 类方法实现
// 清空所有棋盘格子
void Chessboard::clearBlocks() { 
    blocks.clear(); 
}

// 添加新的棋盘格子
void Chessboard::addBlock(int x, int y) {
    blocks.emplace_back(x, y, BLOCK_SIZE, BLOCK_SIZE);
}

// 设置指定索引格子的棋子状态
void Chessboard::setPieceAt(int index, bool hasPiece) {
    if (index >= 0 && index < blocks.size()) {
        blocks[index].setPiece(hasPiece);
    }
}

// 设置指定索引格子的目标状态
void Chessboard::setTargetAt(int index, bool isTarget) {
    if (index >= 0 && index < blocks.size()) {
        blocks[index].setTarget(isTarget);
    }
}

// 清除所有格子的目标状态
void Chessboard::clearAllTargets() {
    for (SingleBlock& block : blocks) {
        block.setTarget(false);
    }
}

// 设置指定索引格子的提示起始状态
void Chessboard::setHintFromAt(int index, bool isHintFrom) {
    if (index >= 0 && index < blocks.size()) {
        blocks[index].setHintFrom(isHintFrom);
    }
}

// 清除所有格子的提示起始状态
void Chessboard::clearAllHintFrom() {
    for (SingleBlock& block : blocks) {
        block.setHintFrom(false);
    }
}

// 获取棋盘格子总数
int Chessboard::getBlockCount() const {
    return blocks.size();
}

// 根据鼠标位置更新格子悬停状态
void Chessboard::updateHover(int mouseX, int mouseY) {
    for (SingleBlock& block : blocks) {
        block.setHovered(block.containsPoint(mouseX, mouseY));
    }
}

// 处理鼠标点击事件
bool Chessboard::handleClick(int mouseX, int mouseY) {
    int clickedIndex = -1;
    
    for (int i = 0; i < blocks.size(); i++) {
        if (blocks[i].containsPoint(mouseX, mouseY)) {
            clickedIndex = i;
            break;
        }
    }
    
    if (clickedIndex == -1) return false;
    
    if (blocks[clickedIndex].getMovable()) {
        return executeMove(clickedIndex);
    }
    
    if (blocks[clickedIndex].containsPiece()) {
        selectPiece(clickedIndex);
        return true;
    }
    
    clearSelection();
    return true;
}

// 选中棋子并显示可移动位置
void Chessboard::selectPiece(int index) {
    clearSelection();
    selectedIndex = index;
    blocks[index].setSelected(true);
    showPossibleMoves(index);
}

// 清除当前选中状态
void Chessboard::clearSelection() {
    selectedIndex = -1;
    for (SingleBlock& block : blocks) {
        block.setSelected(false);
        block.setMovable(false);
    }
}

// 显示指定棋子的可移动位置
void Chessboard::showPossibleMoves(int fromIndex) {
    if (fromIndex < 0 || fromIndex >= blocks.size()) return;
    
    int fromX = blocks[fromIndex].getX();
    int fromY = blocks[fromIndex].getY();
    
    int directions[4][2] = {{0, -70}, {0, 70}, {-70, 0}, {70, 0}};
    
    for (int i = 0; i < 4; i++) {
        int middleX = fromX + directions[i][0];
        int middleY = fromY + directions[i][1];
        int toX = fromX + 2 * directions[i][0];
        int toY = fromY + 2 * directions[i][1];
        
        int middleIndex = this->findBlockAt(middleX, middleY);
        int toIndex = this->findBlockAt(toX, toY);
        
        if (middleIndex != -1 && toIndex != -1) {
            if (blocks[middleIndex].containsPiece() && !blocks[toIndex].containsPiece()) {
                blocks[toIndex].setMovable(true);
            }
        }
    }
}

// 根据坐标查找对应的格子索引
int Chessboard::findBlockAt(int x, int y) const {
    for (int i = 0; i < blocks.size(); i++) {
        if (blocks[i].getX() == x && blocks[i].getY() == y) {
            return i;
        }
    }
    return -1;
}

// 执行棋子移动操作
bool Chessboard::executeMove(int toIndex) {
    if (selectedIndex == -1 || toIndex < 0 || toIndex >= blocks.size()) return false;
    
    int fromX = blocks[selectedIndex].getX();
    int fromY = blocks[selectedIndex].getY();
    int toX = blocks[toIndex].getX();
    int toY = blocks[toIndex].getY();
    
    int middleX = (fromX + toX) / 2;
    int middleY = (fromY + toY) / 2;
    
    int middleIndex = findBlockAt(middleX, middleY);
    
    if (middleIndex == -1) return false;
    
    moveHistory.push(MoveRecord(selectedIndex, middleIndex, toIndex));
    
    blocks[selectedIndex].setPiece(false);
    blocks[middleIndex].setPiece(false);
    blocks[toIndex].setPiece(true);
    
    // 棋局已改变，清除所有目标标记和提示起始标记
    clearAllTargets();
    clearAllHintFrom();
      clearSelection();
    return true;
}

// 查询指定索引处是否有棋子
bool Chessboard::hasPieceAt(int index) const {
    if (index >= 0 && index < blocks.size()) {
        return blocks[index].containsPiece();
    }    return false;
}

// 生成所有可能的反向移动
std::vector<MoveRecord> Chessboard::getReverseMoves() const {
    std::vector<MoveRecord> revs;
    int dirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    for (int i = 0; i < blocks.size(); ++i) {
        if (!blocks[i].containsPiece()) continue;
        int x = blocks[i].getX();
        int y = blocks[i].getY();
        for (auto& d : dirs) {
            int middleX = x + d[0] * BLOCK_SIZE;
            int middleY = y + d[1] * BLOCK_SIZE;
            int fromX = x + d[0] * 2 * BLOCK_SIZE;
            int fromY = y + d[1] * 2 * BLOCK_SIZE;
            int middleIndex = this->findBlockAt(middleX, middleY);
            int fromIndex = this->findBlockAt(fromX, fromY);
            if (middleIndex != -1 && fromIndex != -1) {
                if (!blocks[middleIndex].containsPiece() && !blocks[fromIndex].containsPiece()) {
                    revs.emplace_back(fromIndex, middleIndex, i);
                }
            }
        }
    }    return revs;
}

// 应用反向移动
void Chessboard::applyReverseMove(const MoveRecord& rec) {
    if (rec.fromIndex >= 0 && rec.fromIndex < blocks.size() &&
        rec.middleIndex >= 0 && rec.middleIndex < blocks.size() &&
        rec.toIndex >= 0 && rec.toIndex < blocks.size()) {
        blocks[rec.fromIndex].setPiece(true);
        blocks[rec.middleIndex].setPiece(true);
        blocks[rec.toIndex].setPiece(false);    }
}

// 渲染整个棋盘
void Chessboard::render() const {
    // 遍历 vector 中的所有格子
    for (const SingleBlock& block : blocks) {
        renderSingleBlock(block);    }
}

// 渲染指定索引的单个格子
void Chessboard::renderBlockAt(int index) const {
    if (index >= 0 && index < blocks.size()) {
        renderSingleBlock(blocks[index]);    }
}

// 渲染单个格子的详细样式
void Chessboard::renderSingleBlock(const SingleBlock& block) const {
    int centerX = block.getX() + block.getWidth() / 2;
    int centerY = block.getY() + block.getHeight() / 2;
    int pieceRadius = block.getWidth() / 3;
    
    // 绘制格子底座（白色圆形凹槽，蓝色边框）
    setfillcolor(RGB(250, 252, 255));  // 纯净白色
    setlinecolor(RGB(70, 130, 220));   // 清爽蓝色边框
    setlinestyle(PS_SOLID, 2);
    solidcircle(centerX, centerY, pieceRadius + 4);
    circle(centerX, centerY, pieceRadius + 4);
      // 内圈阴影效果（浅蓝色）
    setfillcolor(RGB(240, 245, 255));  // 极浅蓝色
    solidcircle(centerX, centerY, pieceRadius + 2);
    
    if (block.containsPiece()) {
        // 根据状态设置不同的棋子样式
        if (block.getSelected()) {
            // 选中状态：鲜艳绿色棋子（对比色突出显示）
            setfillcolor(RGB(50, 205, 50));   // 鲜艳绿色
            solidcircle(centerX, centerY, pieceRadius);
            // 高光效果（白色）
            setfillcolor(RGB(255, 255, 255));
            solidcircle(centerX - 8, centerY - 8, pieceRadius / 3);
            // 深色边框增强对比度
            setlinecolor(RGB(34, 139, 34));
            setlinestyle(PS_SOLID, 2);
            circle(centerX, centerY, pieceRadius);
            // 选中光环（鲜艳绿色）
            setlinecolor(RGB(50, 205, 50));
            setlinestyle(PS_SOLID, 3);
            circle(centerX, centerY, pieceRadius + 8);
        } else if (block.getHovered()) {
            // 悬停状态：亮蓝色棋子
            setfillcolor(RGB(50, 110, 200));  // 中蓝色
            solidcircle(centerX, centerY, pieceRadius);
            // 高光效果（白色）
            setfillcolor(RGB(255, 255, 255));
            solidcircle(centerX - 6, centerY - 6, pieceRadius / 3);
            // 深蓝色边框增强对比度
            setlinecolor(RGB(20, 50, 120));
            setlinestyle(PS_SOLID, 2);
            circle(centerX, centerY, pieceRadius);
            // 悬停光环（中蓝色）
            setlinecolor(RGB(50, 110, 200));
            setlinestyle(PS_SOLID, 2);
            circle(centerX, centerY, pieceRadius + 6);        } else {
            // 正常状态：标准蓝色棋子，深蓝色边框
            setfillcolor(RGB(70, 130, 220));  // 标准蓝色
            solidcircle(centerX, centerY, pieceRadius);
            // 高光效果（白色）
            setfillcolor(RGB(255, 255, 255));
            solidcircle(centerX - 6, centerY - 6, pieceRadius / 3);
            // 深蓝色边框增强对比度
            setlinecolor(RGB(20, 50, 120));
            setlinestyle(PS_SOLID, 2);
            circle(centerX, centerY, pieceRadius);
        }
          // 如果是提示起始位置，添加黄色光环
        if (block.getHintFrom()) {
            setlinecolor(RGB(255, 215, 0));  // 金黄色
            setlinestyle(PS_SOLID, 3);       // 加粗光环
            circle(centerX, centerY, pieceRadius + 8);
        }
    } else {
        // 没有棋子时的状态
        if (block.getTarget()) {
            // 目标位置：金色指示器
            
            // 将整个格子底座改为金色背景
            setfillcolor(RGB(255, 215, 100));  // 浅金色背景
            solidcircle(centerX, centerY, pieceRadius + 4);
            
            // 金色边框，闪亮效果
            setlinecolor(RGB(255, 215, 0));    // 标准金色边框
            setlinestyle(PS_SOLID, 4);         // 加粗边框使其更明显
            circle(centerX, centerY, pieceRadius + 4);
            
            // 内圈金色阴影效果
            setfillcolor(RGB(255, 235, 150));  // 更浅的金色
            solidcircle(centerX, centerY, pieceRadius + 2);
            
            // 中心金色星形指示器
            setfillcolor(RGB(255, 215, 0));    // 鲜艳金色
            solidcircle(centerX, centerY, pieceRadius * 2 / 3);
            
            // 白色高光效果，使其更加闪亮
            setfillcolor(RGB(255, 255, 255));
            solidcircle(centerX - 6, centerY - 6, pieceRadius / 3);
            
            // 深金色边框增强对比度
            setlinecolor(RGB(184, 134, 11));
            setlinestyle(PS_SOLID, 2);
            circle(centerX, centerY, pieceRadius * 2 / 3);
            
            // 外围闪光效果（可选）
            setlinecolor(RGB(255, 215, 0));
            setlinestyle(PS_SOLID, 2);
            circle(centerX, centerY, pieceRadius + 8);
        } else if (block.getMovable()) {
            // 可移动位置：橙红色指示器
            
            // 将整个格子底座改为橙红色背景
            setfillcolor(RGB(255, 140, 100));  // 浅橙红色背景
            solidcircle(centerX, centerY, pieceRadius + 4);
            
            // 橙红色边框，与棋盘大小相等
            setlinecolor(RGB(255, 69, 0));     // 鲜艳橙红色边框
            setlinestyle(PS_SOLID, 4);         // 加粗边框使其更明显
            circle(centerX, centerY, pieceRadius + 4);
            
            // 内圈橙红色阴影效果
            setfillcolor(RGB(255, 180, 150));  // 更浅的橙红色
            solidcircle(centerX, centerY, pieceRadius + 2);
            
            // 中心橙红色圆形指示器
            setfillcolor(RGB(255, 69, 0));     // 鲜艳橙红色
            solidcircle(centerX, centerY, pieceRadius * 2 / 3);
            
            // 白色高光效果
            setfillcolor(RGB(255, 255, 255));
            solidcircle(centerX - 6, centerY - 6, pieceRadius / 4);
            
            // 深橙红色边框增强对比度
            setlinecolor(RGB(200, 50, 0));
            setlinestyle(PS_SOLID, 2);
            circle(centerX, centerY, pieceRadius * 2 / 3);
        } else if (block.getHovered()) {
            // 空位悬停：极浅蓝色提示
            setfillcolor(RGB(220, 235, 255));  // 极浅蓝色
            solidcircle(centerX, centerY, pieceRadius / 4);
            setlinecolor(RGB(120, 170, 240));
            setlinestyle(PS_SOLID, 1);
            circle(centerX, centerY, pieceRadius / 2);
        }    }
}

// 检查游戏是否胜利
bool Chessboard::isGameWon() const {
    int pieceCount = 0;
    for (const SingleBlock& block : blocks) {
        if (block.containsPiece()) {
            pieceCount++;
        }
    }    return pieceCount == 1; // 只剩一个棋子时胜利
}

// 检查游戏是否失败
bool Chessboard::isGameLost() const {
    // 检查是否有任何棋子可以移动
    for (int i = 0; i < blocks.size(); i++) {
        if (blocks[i].containsPiece() && canPieceMove(i)) {
            return false; // 找到可移动的棋子，游戏未失败
        }
    }    return true; // 没有棋子可以移动，游戏失败
}

// 检查指定棋子是否可以移动
bool Chessboard::canPieceMove(int index) const {
    if (index < 0 || index >= blocks.size() || !blocks[index].containsPiece()) {
        return false;
    }
    
    // 获取棋子位置
    int fromX = blocks[index].getX();
    int fromY = blocks[index].getY();
    
    // 检查四个方向（上下左右）
    int directions[4][2] = {{0, -70}, {0, 70}, {-70, 0}, {70, 0}}; // 基于BLOCK_SIZE=70
    
    for (int i = 0; i < 4; i++) {
        int middleX = fromX + directions[i][0];
        int middleY = fromY + directions[i][1];
        int toX = fromX + 2 * directions[i][0];
        int toY = fromY + 2 * directions[i][1];
        
        // 查找中间位置和目标位置的索引
        int middleIndex = this->findBlockAt(middleX, middleY);
        int toIndex = this->findBlockAt(toX, toY);
        
        // 如果找到了有效的中间位置和目标位置
        if (middleIndex != -1 && toIndex != -1) {
            // 中间有棋子，目标位置为空
            if (blocks[middleIndex].containsPiece() && !blocks[toIndex].containsPiece()) {
                return true; // 找到一个可移动的方向
            }
        }
    }
      return false; // 没有可移动的方向
}

// 执行悔棋操作
bool Chessboard::undoMove() {
    if (moveHistory.empty()) return false;
    
    // 获取最近的移动记录
    MoveRecord lastMove = moveHistory.top();
    moveHistory.pop();
      // 逆向操作：恢复棋盘状态
    blocks[lastMove.fromIndex].setPiece(true);      // 恢复起始位置的棋子
    blocks[lastMove.middleIndex].setPiece(true);    // 恢复被吃掉的棋子
    blocks[lastMove.toIndex].setPiece(false);       // 清空目标位置    // 棋局已改变，清除所有目标标记和提示起始标记
    clearAllTargets();
    clearAllHintFrom();
    
    clearSelection();  // 清除当前选择状态
    return true;
}

// 检查是否可以悔棋
bool Chessboard::canUndo() const {
    return !moveHistory.empty();
}

// 清空移动历史记录
void Chessboard::clearHistory() {
    // 清空移动历史记录栈
    while (!moveHistory.empty()) {
        moveHistory.pop();
    }
}

// 渲染棋盘背景图片
void renderImage(const TCHAR* imageName, const TCHAR* displayName) {
    static bool imageLoaded = false;
    static IMAGE boardImage;
    static TCHAR lastImageName[MAX_PATH] = _T("");
    
    // 检查是否需要重新加载图片（图片名称不同或第一次加载）
    if (!imageLoaded || _tcscmp(lastImageName, imageName) != 0) {
        loadimage(&boardImage, imageName);
        _tcscpy_s(lastImageName, imageName);
        imageLoaded = true;
    }
    
    // 获取图片尺寸
    int imgWidth = boardImage.getwidth();
    int imgHeight = boardImage.getheight();
    
    // 检查图片是否有效（宽度和高度大于0）
    if (imgWidth > 0 && imgHeight > 0) {
        // 在图片上方添加说明文字
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
        
        const TCHAR* label = displayName;
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
        
        const TCHAR* errorText = _T("图片加载失败，请检查文件路径");
        int textWidth = textwidth(errorText);
        outtextxy(640 - textWidth/2, 350, errorText);
    }
}
