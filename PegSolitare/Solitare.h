#ifndef SOLITARE_H
#define SOLITARE_H

#include <vector>
#include <easyx.h>
#include <tchar.h>
#include <string>
#include <unordered_map>
#include <stack>
#include "BoardsConfig.h"
using namespace std;

// 前向声明
class MainMenuState;
class ChooseGameState;
class HowToPlayState;
class ExitState;
class GameState;
class ContinueGameState;
class GameFailedState;
class GameWonState;
class RestartConfirmState;

// 全局对象声明
extern MainMenuState mainMenu;
extern ChooseGameState chooseGame;
extern HowToPlayState howToPlay;
extern ExitState   exitState;
extern GameState   gameState;
extern ContinueGameState continueGameState;
extern GameFailedState gameFailedState;
extern GameWonState gameWonState;
extern RestartConfirmState restartConfirmState;

// 移动记录结构体
struct MoveRecord {
    int fromIndex;
    int middleIndex;
    int toIndex;
    
    MoveRecord(int from, int middle, int to) 
        : fromIndex(from), middleIndex(middle), toIndex(to) {}
};

class Button {
private:
    int x, y, width, height;
    const TCHAR* text;
    COLORREF fillColor;
    COLORREF borderColor;
    COLORREF textColor;
    bool enabled;

public:
    Button(int x, int y, int width, int height, const TCHAR* text, 
           COLORREF fill = RGB(0, 120, 215), COLORREF border = RGB(0, 84, 153), COLORREF textCol = WHITE, bool isEnabled = true);
    void draw() const;
    void drawWithHover(int mouseX, int mouseY) const;
    bool isClicked(int mouseX, int mouseY) const;
    bool isHovered(int mouseX, int mouseY) const;
    void setEnabled(bool isEnabled);
    bool getEnabled() const;
};


class SingleBlock {
private:
    int x, y;
    int width, height;
    bool hasPiece;
    bool isHovered;
    bool isSelected;
    bool isMovable;
    bool isTarget;
    bool isHintFrom;
public:
    SingleBlock(int _x, int _y, int _w, int _h);
    
    // 棋子状态
    bool containsPiece() const;
    void setPiece(bool val);
    
    // 鼠标悬停
    bool getHovered() const;
    void setHovered(bool hovered);
    
    // 选中状态
    bool getSelected() const;
    void setSelected(bool selected);
    
    // 可移动状态
    bool getMovable() const;
    void setMovable(bool movable);
    
    // 目标状态
    bool getTarget() const;
    void setTarget(bool target);
    
    // 提示起始位置状态
    bool getHintFrom() const;
    void setHintFrom(bool hintFrom);
    
    // 检测点是否在格子内
    bool containsPoint(int px, int py) const;
    
    // 获取坐标和尺寸
    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
};

// 棋盘类
class Chessboard {
private:
    vector<SingleBlock> blocks;
    const int BLOCK_SIZE = 70;
    int selectedIndex = -1;
    stack<MoveRecord> moveHistory;
    
    // 私有方法
    void selectPiece(int index);
    void clearSelection();
    void showPossibleMoves(int fromIndex);
    bool executeMove(int toIndex);
    void renderSingleBlock(const SingleBlock& block) const;

public:
    // 查找格子索引
    int findBlockAt(int x, int y) const;
    
    // 公有方法
    void clearBlocks();
    void addBlock(int x, int y);
    void setPieceAt(int index, bool hasPiece);
    int getBlockCount() const;
    void updateHover(int mouseX, int mouseY);
    bool handleClick(int mouseX, int mouseY);
    void render() const;
    void renderBlockAt(int index) const;
    
    // 查询是否有棋子
    bool hasPieceAt(int index) const;
    
    // 目标位置标记
    void setTargetAt(int index, bool isTarget);
    void clearAllTargets();
    
    // 提示起始位置标记
    void setHintFromAt(int index, bool isHintFrom);
    void clearAllHintFrom();
    
    // 生成所有可能的反向移动
    std::vector<MoveRecord> getReverseMoves() const;
    // 反向移动
    void applyReverseMove(const MoveRecord& rec);
    
    // 胜负检测方法
    bool isGameWon() const;
    bool isGameLost() const;
    bool canPieceMove(int index) const;
    
    // 悔棋相关方法
    bool undoMove();
    bool canUndo() const;
    void clearHistory();
};

// 状态节点类
class StateNode {
public:
    virtual void render() = 0;
    virtual StateNode* handleEvent() = 0;
    virtual ~StateNode() = default;
};

// 通用确认对话框基类
class ConfirmBase : public StateNode {
protected:
    // 外观配置
    COLORREF backgroundColor;
    COLORREF dialogColor;
    COLORREF borderColor;
    COLORREF titleColor;
    COLORREF contentColor;
    COLORREF hintColor;
    
    // 文本内容
    const TCHAR* titleText;
    const TCHAR* contentText;
    const TCHAR* hintText;
    
    // 按钮配置
    COLORREF yesButtonColor;
    COLORREF noButtonColor;
    const TCHAR* yesButtonText;
    const TCHAR* noButtonText;
    
    // 按钮对象
    Button yesButton;
    Button noButton;
    
public:
    // 构造函数
    ConfirmBase(
        const TCHAR* title, 
        const TCHAR* content, 
        const TCHAR* hint,
        COLORREF bgColor = RGB(240, 245, 250),
        COLORREF dlgColor = WHITE,
        COLORREF borderClr = RGB(0, 84, 153),
        COLORREF titleClr = RGB(0, 84, 153),
        COLORREF contentClr = RGB(0, 84, 153),
        COLORREF hintClr = RGB(128, 128, 128),
        COLORREF yesClr = RGB(0, 120, 215),
        COLORREF noClr = RGB(108, 117, 125),
        const TCHAR* yesText = _T("是"),
        const TCHAR* noText = _T("否")
    );    // 通用渲染方法
    virtual void render() override;
};

// 标题类，用于绘制大字号居中标题
class Title {
private:
    const TCHAR* text;
    int fontSize;
    int y;
public:
    Title(const TCHAR* txt, int sz, int ypos) : text(txt), fontSize(sz), y(ypos) {}
    void draw() const;
};

// 主菜单状态类
class MainMenuState : public StateNode {
private:
    Title pageTitle = Title(_T("孔明棋"), 70, 25);
    Button startButton = Button(560, 240, 160, 50, _T("开始游戏"));
    Button howToPlayButton = Button(560, 330, 160, 50, _T("玩法介绍"));
    Button exitButton = Button(560, 420, 160, 50, _T("退出游戏"));
public:
    void render() override;
    StateNode* handleEvent() override;
};

// 选择游戏状态类
class ChooseGameState : public StateNode {
private:
    Title pageTitle = Title(_T("选择游戏"), 60, 25);
    Button returnButton = Button(20, 30, 100, 40, _T("返回"));
    Button startButton = Button(450, 580, 160, 50, _T("经典模式"));
    Button endgameButton = Button(670, 580, 160, 50, _T("残局模式"));
    bool pendingEndgame = false;
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
class ExitState : public ConfirmBase {
public:
    ExitState();
    StateNode* handleEvent() override;
};

// 继续游戏确认状态类
class ContinueGameState : public ConfirmBase {
public:
    ContinueGameState();
    StateNode* handleEvent() override;
};

// 重新开始确认状态类
class RestartConfirmState : public ConfirmBase {
public:
    RestartConfirmState();
    StateNode* handleEvent() override;
};

// 游戏失败状态类
class GameFailedState : public ConfirmBase {
public:
    GameFailedState();
    StateNode* handleEvent() override;
};

// 游戏胜利状态类
class GameWonState : public ConfirmBase {
public:
    GameWonState();
    StateNode* handleEvent() override;
};

class GameState : public StateNode {
private:
    Chessboard board;
    bool boardInitialized = false;
    bool gameStarted = false;
    bool endgameMode = false;
    Title pageTitle;
    Button returnButton;
    Button undoButton;
    Button restartButton;
    Button hintButton;
    
    // 状态文本相关
    const TCHAR* statusText = _T("");
    COLORREF statusTextColor = RGB(255, 255, 255);
    
    // 图例渲染方法
    void renderLegend() const;
    void renderLegendPiece(int x, int y, int radius, COLORREF fillColor, COLORREF borderColor, bool hasHighlight = true) const;
    void renderLegendMovable(int x, int y, int radius) const;
    void renderLegendTarget(int x, int y, int radius) const;
    
    // 状态文本方法
    void setStatusText(const TCHAR* text, COLORREF color = RGB(255, 255, 255));
    void renderStatusText() const;
    
public:
    GameState() : pageTitle(_T("游戏中"),60,25), returnButton(20,30,100,40,_T("返回")), undoButton(1150,350,100,40,_T("悔棋")), restartButton(1150,400,100,40,_T("重新开始"), RGB(255, 140, 0), RGB(230, 120, 0), WHITE), hintButton(1150,300,100,40,_T("提示"), RGB(0, 120, 215), RGB(0, 84, 153), WHITE) {}
    void render() override;
    StateNode* handleEvent() override;
    void BoardInit(const std::string& boardName);
    
    // 游戏进度管理方法
    bool isGameStarted() const { return gameStarted; }
    void resetGame();
    void startEndgame();
    bool isEndgameMode() const { return endgameMode; }
    void restartCurrentMode();
    
    // 悔棋相关方法
    Chessboard& getBoard() { return board; }
};


extern MainMenuState mainMenu;
extern ChooseGameState chooseGame;
extern HowToPlayState howToPlay;
extern ExitState exitState;
extern GameState gameState;
extern GameFailedState gameFailedState;
extern GameWonState gameWonState;
extern RestartConfirmState restartConfirmState;

void init();

// 全局图片渲染函数
void renderImage(const TCHAR* imageName, const TCHAR* displayName);

#endif // SOLITARE_H
