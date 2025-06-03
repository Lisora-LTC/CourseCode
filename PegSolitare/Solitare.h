#ifndef SOLITARE_H
#define SOLITARE_H

#include <vector>
#include <easyx.h>
#include <tchar.h>
#include <string>
#include <unordered_map> // 添加头文件
using namespace std;

class Button {
private:
    int x, y, width, height;
    const TCHAR* text; // 修改为 const TCHAR*

public:
    Button(int x, int y, int width, int height, const TCHAR* text); // 修改构造函数
    void draw() const;
    bool isClicked(int mouseX, int mouseY) const;//const 用来表示不会修改变量，只读
};

class Chess {
    // 类内部暂时不需要定义
};

// 状态枚举
enum GameState {
    MAIN_MENU,
    CHOOSE_GAME,
    EXIT
};

// 状态节点类
class StateNode {
public:
    virtual void render() = 0; // 渲染页面
    virtual void handleEvent(GameState& currentState) = 0; // 处理事件并切换状态
};

// 主菜单状态类
class MainMenuState : public StateNode {
public:
    void render() override;
    void handleEvent(GameState& currentState) override;
private:
    Button startButton = Button(560, 300, 160, 50, _T("开始游戏"));
    Button exitButton = Button(560, 400, 160, 50, _T("退出游戏"));
};

// 选择游戏状态类
class ChooseGameState : public StateNode {
private:
    Button returnButton = Button(20, 20, 100, 40, _T("返回"));
public:
    void render() override;
    void handleEvent(GameState& currentState) override;
};

// 退出状态类
class ExitState : public StateNode {
private:
    Button yesButton = Button(520, 360, 100, 40, _T("是"));
    Button noButton = Button(680, 360, 100, 40, _T("否"));

public:
    void render() override;
    void handleEvent(GameState& currentState) override;
};

void init();       // 初始化图形界面

extern MainMenuState mainMenu;
extern ChooseGameState chooseGame;
extern ExitState exitState;

// Global state map declaration
extern unordered_map<GameState, StateNode*> stateMap;

#endif // SOLITARE_H
