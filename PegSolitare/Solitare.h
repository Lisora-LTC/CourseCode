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
    void draw() const {
        // 设置字体样式并测量宽度居中显示
        settextstyle(fontSize, 0, _T("微软雅黑"));
        int w = textwidth(text);
        settextcolor(WHITE);
        setbkmode(TRANSPARENT);
        outtextxy(1280/2 - w/2, y, text);
    }
};

// 主菜单状态类
class MainMenuState : public StateNode {
public:
    void render() override;
    StateNode* handleEvent() override;
private:
    Title pageTitle = Title(_T("孔明棋"), 80, 30);
    Button startButton = Button(560, 300, 160, 50, _T("开始游戏"));
    Button exitButton = Button(560, 400, 160, 50, _T("退出游戏"));
};

// 选择游戏状态类
class ChooseGameState : public StateNode {
private:
    Title pageTitle = Title(_T("选择游戏"), 60, 30);
    Button returnButton = Button(20, 20, 100, 40, _T("返回"));
public:
    void render() override;
    StateNode* handleEvent() override;
};

// 退出状态类
class ExitState : public StateNode {
private:
    Button yesButton = Button(520, 360, 100, 40, _T("是"));
    Button noButton = Button(680, 360, 100, 40, _T("否"));

public:
    void render() override;
    StateNode* handleEvent() override;
};

void init();       // 初始化图形界面

extern MainMenuState mainMenu;
extern ChooseGameState chooseGame;
extern ExitState exitState;

#endif // SOLITARE_H
