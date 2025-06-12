#include "Solitare.h"
#include <cmath>    // 数学函数如 sqrt, sin, cos
#include <algorithm> // min, max 函数

// Title 类方法实现
void Title::draw() const {
    // 设置字体样式并测量宽度居中显示 (启用抗锯齿)
    LOGFONT font;
    gettextstyle(&font);
    font.lfHeight = fontSize; // 恢复原始字体大小
    font.lfWidth = 0;
    font.lfWeight = FW_NORMAL;
    font.lfQuality = ANTIALIASED_QUALITY; // 保留抗锯齿
    _tcscpy_s(font.lfFaceName, _T("微软雅黑"));
    settextstyle(&font);
    
    int w = textwidth(text);
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    outtextxy(1280/2 - w/2, y, text); // 恢复原始坐标
}

// SingleBlock 类方法实现
SingleBlock::SingleBlock(int _x, int _y, int _w, int _h)
    : x(_x), y(_y), width(_w), height(_h), hasPiece(false), isHovered(false), isSelected(false), isMovable(false) {}
bool SingleBlock::containsPiece() const { return hasPiece; }
void SingleBlock::setPiece(bool val) { hasPiece = val; }
bool SingleBlock::getHovered() const { return isHovered; }
void SingleBlock::setHovered(bool hovered) { isHovered = hovered; }
bool SingleBlock::getSelected() const { return isSelected; }
void SingleBlock::setSelected(bool selected) { isSelected = selected; }
bool SingleBlock::getMovable() const { return isMovable; }
void SingleBlock::setMovable(bool movable) { isMovable = movable; }
bool SingleBlock::containsPoint(int px, int py) const { return px >= x && px <= x + width && py >= y && py <= y + height; }
int SingleBlock::getX() const { return x; }
int SingleBlock::getY() const { return y; }
int SingleBlock::getWidth() const { return width; }
int SingleBlock::getHeight() const { return height; }

// Chessboard 类方法实现

// 清除所有格子
void Chessboard::clearBlocks() { 
    blocks.clear(); 
}

// 添加格子的方法，传入左上角坐标
void Chessboard::addBlock(int x, int y) {
    blocks.emplace_back(x, y, BLOCK_SIZE, BLOCK_SIZE);
}

// 设置指定索引格子的棋子状态
void Chessboard::setPieceAt(int index, bool hasPiece) {
    if (index >= 0 && index < blocks.size()) {
        blocks[index].setPiece(hasPiece);
    }
}

// 获取格子总数
int Chessboard::getBlockCount() const {
    return blocks.size();
}

// 更新鼠标悬停状态
void Chessboard::updateHover(int mouseX, int mouseY) {
    for (SingleBlock& block : blocks) {
        block.setHovered(block.containsPoint(mouseX, mouseY));
    }
}

// 处理鼠标点击，返回是否有状态改变
bool Chessboard::handleClick(int mouseX, int mouseY) {
    int clickedIndex = -1;
    
    // 找到被点击的格子
    for (int i = 0; i < blocks.size(); i++) {
        if (blocks[i].containsPoint(mouseX, mouseY)) {
            clickedIndex = i;
            break;
        }
    }
    
    if (clickedIndex == -1) return false;
    
    // 检查是否点击了可移动的位置
    if (blocks[clickedIndex].getMovable()) {
        return executeMove(clickedIndex);
    }
    
    // 检查是否点击了有棋子的格子
    if (blocks[clickedIndex].containsPiece()) {
        selectPiece(clickedIndex);
        return true;
    }
    
    // 点击空格子，清除所有选择
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

// 清除所有选择状态
void Chessboard::clearSelection() {
    selectedIndex = -1;
    for (SingleBlock& block : blocks) {
        block.setSelected(false);
        block.setMovable(false);
    }
}

// 显示可能的移动位置 - 基于实际坐标而非网格
void Chessboard::showPossibleMoves(int fromIndex) {
    if (fromIndex < 0 || fromIndex >= blocks.size()) return;
    
    // 获取起始位置的实际坐标
    int fromX = blocks[fromIndex].getX();
    int fromY = blocks[fromIndex].getY();
    
    // 检查四个方向（上下左右）
    int directions[4][2] = {{0, -70}, {0, 70}, {-70, 0}, {70, 0}}; // 基于BLOCK_SIZE=70
    
    for (int i = 0; i < 4; i++) {
        int middleX = fromX + directions[i][0];
        int middleY = fromY + directions[i][1];
        int toX = fromX + 2 * directions[i][0];
        int toY = fromY + 2 * directions[i][1];
        
        // 查找中间位置和目标位置的索引
        int middleIndex = findBlockAt(middleX, middleY);
        int toIndex = findBlockAt(toX, toY);
        
        // 如果找到了有效的中间位置和目标位置
        if (middleIndex != -1 && toIndex != -1) {
            // 中间有棋子，目标位置为空
            if (blocks[middleIndex].containsPiece() && !blocks[toIndex].containsPiece()) {
                blocks[toIndex].setMovable(true);
            }
        }
    }
}

// 查找指定坐标处的格子索引
int Chessboard::findBlockAt(int x, int y) const {
    for (int i = 0; i < blocks.size(); i++) {
        if (blocks[i].getX() == x && blocks[i].getY() == y) {
            return i;
        }
    }
    return -1;
}

// 执行移动 - 基于实际坐标
bool Chessboard::executeMove(int toIndex) {
    if (selectedIndex == -1 || toIndex < 0 || toIndex >= blocks.size()) return false;
    
    // 获取起始和目标位置的坐标
    int fromX = blocks[selectedIndex].getX();
    int fromY = blocks[selectedIndex].getY();
    int toX = blocks[toIndex].getX();
    int toY = blocks[toIndex].getY();
    
    // 计算中间位置坐标
    int middleX = (fromX + toX) / 2;
    int middleY = (fromY + toY) / 2;
    
    // 查找中间位置的索引
    int middleIndex = findBlockAt(middleX, middleY);
    
    if (middleIndex == -1) return false;
    
    // 执行移动
    blocks[selectedIndex].setPiece(false);  // 起始位置清空
    blocks[middleIndex].setPiece(false);    // 中间棋子被吃掉
    blocks[toIndex].setPiece(true);         // 目标位置放置棋子
    
    clearSelection();
    return true;
}

// 渲染整个棋盘的方法
void Chessboard::render() const {
    // 遍历 vector 中的所有格子
    for (const SingleBlock& block : blocks) {
        renderSingleBlock(block);
    }
}

// 只渲染指定索引的单个格子
void Chessboard::renderBlockAt(int index) const {
    if (index >= 0 && index < blocks.size()) {
        renderSingleBlock(blocks[index]);
    }
}

// 渲染单个格子的通用方法 - 蓝白配色主题
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
    solidcircle(centerX, centerY, pieceRadius + 2);    if (block.containsPiece()) {
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
            circle(centerX, centerY, pieceRadius + 6);
        } else {
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
    } else {
        // 没有棋子时的状态
        if (block.getMovable()) {
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
        }
    }
}

// 胜负检测方法实现
bool Chessboard::isGameWon() const {
    int pieceCount = 0;
    for (const SingleBlock& block : blocks) {
        if (block.containsPiece()) {
            pieceCount++;
        }
    }
    return pieceCount == 1; // 只剩一个棋子时胜利
}

bool Chessboard::isGameLost() const {
    // 检查是否有任何棋子可以移动
    for (int i = 0; i < blocks.size(); i++) {
        if (blocks[i].containsPiece() && canPieceMove(i)) {
            return false; // 找到可移动的棋子，游戏未失败
        }
    }
    return true; // 没有棋子可以移动，游戏失败
}

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
        int middleIndex = findBlockAt(middleX, middleY);
        int toIndex = findBlockAt(toX, toY);
        
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
