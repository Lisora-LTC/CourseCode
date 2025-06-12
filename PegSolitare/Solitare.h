#ifndef SOLITARE_H
#define SOLITARE_H

#include <vector>
#include <easyx.h>
#include <tchar.h>
#include <string>
#include <unordered_map> // 添加头文件
#include "BoardsConfig.h" // 引入 BoardsConfig
using namespace std;

// 1. 前向声明
class MainMenuState;
class ChooseGameState;
class ExitState;
class GameState;
class ContinueGameState;

// 2. extern 全局对象声明
extern MainMenuState mainMenu;
extern ChooseGameState chooseGame;
extern ExitState   exitState;
extern GameState   gameState;
extern ContinueGameState continueGameState;

class Button {
private:
    int x, y, width, height;
    const TCHAR* text; // 修改为 const TCHAR*

public:
    Button(int x, int y, int width, int height, const TCHAR* text); // 修改构造函数
    void draw() const;
    bool isClicked(int mouseX, int mouseY) const;//const 用来表示不会修改变量，只读
};


class SingleBlock {
private:
    int x, y;         // 左上角坐标
    int width, height; // 格子尺寸
    bool hasPiece;     // 是否有棋子
    bool isHovered;    // 是否被鼠标悬停
    bool isSelected;   // 是否被选中（红色状态）
    bool isMovable;    // 是否可移动到（蓝色状态）
public:
    // 构造函数：设置坐标、尺寸，初始无棋子
    SingleBlock(int _x, int _y, int _w, int _h);
    
    // 棋子状态相关方法
    bool containsPiece() const;
    void setPiece(bool val);
    
    // 鼠标悬停相关方法
    bool getHovered() const;
    void setHovered(bool hovered);
    
    // 选中状态相关方法
    bool getSelected() const;
    void setSelected(bool selected);
    
    // 可移动状态相关方法
    bool getMovable() const;
    void setMovable(bool movable);
    
    // 检测点是否在格子内
    bool containsPoint(int px, int py) const;
    
    // 获取坐标和尺寸的方法
    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
};

// 棋盘类，使用 vector 存储 SingleBlock 对象
class Chessboard {
private:
    vector<SingleBlock> blocks; // 格子容器
    const int BLOCK_SIZE = 70;  // 缩小格子尺寸适配720p
    int selectedIndex = -1; // 当前选中的棋子索引
    
    // 私有方法声明
    void selectPiece(int index);
    void clearSelection();
    void showPossibleMoves(int fromIndex);
    int findBlockAt(int x, int y) const;
    bool executeMove(int toIndex);
    void renderSingleBlock(const SingleBlock& block) const;

public:
    // 公有方法声明
    void clearBlocks();
    void addBlock(int x, int y);
    void setPieceAt(int index, bool hasPiece);
    int getBlockCount() const;    
    void updateHover(int mouseX, int mouseY);
    bool handleClick(int mouseX, int mouseY);
    void render() const;
    void renderBlockAt(int index) const;
    
    // 胜负检测方法
    bool isGameWon() const;      // 检测是否胜利（只剩一个棋子）
    bool isGameLost() const;     // 检测是否失败（无路可走）
    bool canPieceMove(int index) const; // 检测指定棋子是否有可移动位置
};

// 状态节点类
class StateNode {
public:
    virtual void render() = 0; // 渲染页面
    virtual StateNode* handleEvent() = 0; // 处理事件并返回下一个状态
    virtual ~StateNode() = default;
};

// 标题类，用于绘制大字号居中标题
class Title {
private:
    const TCHAR* text;
    int fontSize;
    int y;
public:
    Title(const TCHAR* txt, int sz, int ypos) : text(txt), fontSize(sz), y(ypos) {}
    void draw() const; // 移到外部实现
};

// 主菜单状态类
class MainMenuState : public StateNode {
private:    Title pageTitle = Title(_T("孔明棋"), 70, 25);  // 720p适配
    Button startButton = Button(560, 280, 160, 50, _T("开始游戏"));  // 居中: (1280-160)/2 = 560
    Button exitButton = Button(560, 380, 160, 50, _T("退出游戏"));   // 下移
public:
    void render() override;
    StateNode* handleEvent() override;
};

// 选择游戏状态类
class ChooseGameState : public StateNode {
private:    Title pageTitle = Title(_T("选择游戏"), 60, 25);  // 720p适配
    Button returnButton = Button(20, 20, 100, 40, _T("返回"));  // 720p适配
    Button startButton = Button(560, 280, 160, 50, _T("开始游戏"));  // 720p适配居中
public:
    StateNode* parent = &mainMenu;
    void render() override;
    StateNode* handleEvent() override;
};

// 退出状态类
class ExitState : public StateNode {
private:
    Button yesButton = Button(490, 390, 100, 40, _T("是"));   // 720p完全对称
    Button noButton = Button(690, 390, 100, 40, _T("否"));  // 720p完全对称

public:
    void render() override;
    StateNode* handleEvent() override;
};

// 继续游戏确认状态类
class ContinueGameState : public StateNode {
private:
    Button yesButton = Button(490, 390, 100, 40, _T("是"));   // 720p完全对称
    Button noButton = Button(690, 390, 100, 40, _T("否"));  // 720p完全对称

public:
    void render() override;
    StateNode* handleEvent() override;
};

class GameState : public StateNode {
private:
    Title pageTitle;
    Button returnButton;
    Chessboard board;
    bool boardInitialized = false;
    bool gameStarted = false;  // 跟踪游戏是否已经开始
public:
    GameState() : pageTitle(_T("游戏中"),60,25), returnButton(20,20,100,40,_T("返回")) {}  // 720p适配
    void render() override;
    StateNode* handleEvent() override;
    // 修改为非内联声明
    void BoardInit(const std::string& boardName);
    
    // 游戏进度管理方法
    bool isGameStarted() const { return gameStarted; }
    void resetGame();  // 重置游戏状态
};


extern MainMenuState mainMenu;
extern ChooseGameState chooseGame;
extern ExitState exitState;
extern GameState gameState;

void init();       // 初始化图形界面

#endif // SOLITARE_H
