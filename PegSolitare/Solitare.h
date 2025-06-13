#ifndef SOLITARE_H
#define SOLITARE_H

#include <vector>
#include <easyx.h>
#include <tchar.h>
#include <string>
#include <unordered_map> // 添加头文件
#include <stack>         // 添加栈头文件用于悔棋功能
#include "BoardsConfig.h" // 引入 BoardsConfig
using namespace std;

// 1. 前向声明
class MainMenuState;
class ChooseGameState;
class HowToPlayState;
class ExitState;
class GameState;
class ContinueGameState;
class GameFailedState;
class GameWonState;

// 2. extern 全局对象声明
extern MainMenuState mainMenu;
extern ChooseGameState chooseGame;
extern HowToPlayState howToPlay;
extern ExitState   exitState;
extern GameState   gameState;
extern ContinueGameState continueGameState;
extern GameFailedState gameFailedState;
extern GameWonState gameWonState;

// 移动记录结构体，用于悔棋功能
struct MoveRecord {
    int fromIndex;     // 起始位置索引
    int middleIndex;   // 被吃掉的棋子位置索引
    int toIndex;       // 目标位置索引
    
    MoveRecord(int from, int middle, int to) 
        : fromIndex(from), middleIndex(middle), toIndex(to) {}
};

class Button {
private:
    int x, y, width, height;
    const TCHAR* text; // 修改为 const TCHAR*
    COLORREF fillColor;    // 按钮填充颜色
    COLORREF borderColor;  // 按钮边框颜色
    COLORREF textColor;    // 文字颜色

public:
    Button(int x, int y, int width, int height, const TCHAR* text, 
           COLORREF fill = RGB(0, 120, 215), COLORREF border = RGB(0, 84, 153), COLORREF textCol = WHITE); // 修改构造函数，添加颜色参数
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
    stack<MoveRecord> moveHistory;  // 移动历史记录栈
    
    // 私有方法声明
    void selectPiece(int index);
    void clearSelection();
    void showPossibleMoves(int fromIndex);
    bool executeMove(int toIndex);
    void renderSingleBlock(const SingleBlock& block) const;

public:
    // 查找指定坐标处的格子索引
    int findBlockAt(int x, int y) const;
    
    // 公有方法声明
    void clearBlocks();
    void addBlock(int x, int y);
    void setPieceAt(int index, bool hasPiece);
    int getBlockCount() const;
    void updateHover(int mouseX, int mouseY);
    bool handleClick(int mouseX, int mouseY);
    void render() const;
    void renderBlockAt(int index) const;
    
    // 查询指定索引处是否有棋子
    bool hasPieceAt(int index) const;
    // 生成所有可能的反向移动
    std::vector<MoveRecord> getReverseMoves() const;
    // 应用反向移动
    void applyReverseMove(const MoveRecord& rec);
    
    // 胜负检测方法
    bool isGameWon() const;      // 检测是否胜利（只剩一个棋子）
    bool isGameLost() const;     // 检测是否失败（无路可走）
    bool canPieceMove(int index) const; // 检测指定棋子是否有可移动位置

    // 悔棋相关方法
    bool undoMove();             // 悔棋方法
    bool canUndo() const;        // 检查是否可以悔棋
    void clearHistory();         // 清空历史记录
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
private:    Title pageTitle = Title(_T("孔明棋"), 70, 25);  // 恢复原来的位置，视觉上更好
    Button startButton = Button(560, 240, 160, 50, _T("开始游戏"));  // 居中: (1280-160)/2 = 560，稍微上移
    Button howToPlayButton = Button(560, 330, 160, 50, _T("玩法介绍"));  // 间距加大10px
    Button exitButton = Button(560, 420, 160, 50, _T("退出游戏"));   // 间距加大30px
public:
    void render() override;
    StateNode* handleEvent() override;
};

// 选择游戏状态类
class ChooseGameState : public StateNode {
private:    Title pageTitle = Title(_T("选择游戏"), 60, 25);  // 保持和主菜单一致的y位置
    Button returnButton = Button(20, 30, 100, 40, _T("返回"));  // 垂直居中：(100-40)/2 = 30
    Button startButton = Button(450, 580, 160, 50, _T("经典模式"));  // 左侧按钮，改名为经典模式，向左移动
    Button endgameButton = Button(670, 580, 160, 50, _T("残局模式"));  // 右侧按钮，向右移动
    bool pendingEndgame = false;  // 标记是否等待开始残局模式
public:
    StateNode* parent = &mainMenu;
    void render() override;
    StateNode* handleEvent() override;
    void setPendingEndgame(bool flag) { pendingEndgame = flag; }
    bool isPendingEndgame() const { return pendingEndgame; }
};

// 玩法介绍状态类
class HowToPlayState : public StateNode {
private:
    Title pageTitle = Title(_T("玩法介绍"), 60, 25);
    Button returnButton = Button(20, 30, 100, 40, _T("返回"));
public:
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

// 游戏失败状态类
class GameFailedState : public StateNode {
private:
    Button exitButton = Button(450, 420, 140, 45, _T("退出游戏"));    // 左侧按钮
    Button continueButton = Button(690, 420, 140, 45, _T("继续游戏")); // 右侧按钮

public:
    void render() override;
    StateNode* handleEvent() override;
};

// 游戏胜利状态类
class GameWonState : public StateNode {
private:
    Button nextGameButton = Button(450, 420, 140, 45, _T("下一局"));    // 左侧按钮
    Button exitButton = Button(690, 420, 140, 45, _T("返回菜单"));      // 右侧按钮

public:
    void render() override;
    StateNode* handleEvent() override;
};

class GameState : public StateNode {
private:
    Chessboard board;
    bool boardInitialized = false;
    bool gameStarted = false;  // 跟踪游戏是否已经开始
    bool endgameMode = false;   // 新增标志：是否残局模式
    Title pageTitle;
    Button returnButton;
    Button undoButton;  // 悔棋按钮
    
    // 图例渲染方法
    void renderLegend() const;
    void renderLegendPiece(int x, int y, int radius, COLORREF fillColor, COLORREF borderColor, bool hasHighlight = true) const;
    void renderLegendMovable(int x, int y, int radius) const;
    
public:
    GameState() : pageTitle(_T("游戏中"),60,25), returnButton(20,30,100,40,_T("返回")), undoButton(1150,340,100,40,_T("悔棋")) {}  // 悔棋按钮放在右侧中间
    void render() override;
    StateNode* handleEvent() override;
    // 修改为非内联声明
    void BoardInit(const std::string& boardName);
    
    // 游戏进度管理方法
    bool isGameStarted() const { return gameStarted; }
    void resetGame();  // 重置游戏状态
    void startEndgame();  // 新增残局模式初始化方法
    
    // 悔棋相关方法
    Chessboard& getBoard() { return board; }  // 提供棋盘访问接口
};


extern MainMenuState mainMenu;
extern ChooseGameState chooseGame;
extern HowToPlayState howToPlay;
extern ExitState exitState;
extern GameState gameState;
extern GameFailedState gameFailedState;
extern GameWonState gameWonState;

void init();       // 初始化图形界面

#endif // SOLITARE_H
