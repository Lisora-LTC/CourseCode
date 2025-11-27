#include "MenuScene.h"
#include <windows.h>

// ============== 构造与析构 ==============
MenuScene::MenuScene() : selectedOption(0), menuRunning(false)
{
    InitMenuItems();
}

MenuScene::~MenuScene()
{
    // TODO: 清理
}

// ============== 主方法 ==============
GameMode MenuScene::Show()
{
    // TODO: 显示菜单并等待用户选择
    // 1. 初始化图形窗口
    // 2. 进入菜单循环
    // 3. 处理输入
    // 4. 渲染菜单
    // 5. 返回选择的模式

    menuRunning = true;

    // while (menuRunning) {
    //     HandleMouseInput();
    //     HandleKeyboardInput();
    //     Render();
    //     Sleep(10);
    // }

    return SINGLE; // 临时返回
}

// ============== 私有方法 ==============
void MenuScene::InitMenuItems()
{
    // TODO: 初始化菜单项
    // 添加各种游戏模式选项

    // 示例：
    // MenuItem item1;
    // item1.text = L"入门版";
    // item1.mode = BEGINNER;
    // item1.x = 300; item1.y = 200;
    // item1.width = 200; item1.height = 50;
    // menuItems.push_back(item1);
}

void MenuScene::Render()
{
    // TODO: 渲染菜单
    // 1. 清空屏幕
    // 2. 绘制标题
    // 3. 绘制所有菜单项
    // 4. 绘制说明文字
    // 5. 刷新屏幕
}

void MenuScene::DrawMenuItem(const MenuItem &item, bool isSelected)
{
    // TODO: 绘制单个菜单项
    // 如果被选中，使用高亮颜色
}

void MenuScene::HandleMouseInput()
{
    // TODO: 处理鼠标输入
    // 1. 检测鼠标位置
    // 2. 高亮鼠标悬停的菜单项
    // 3. 检测鼠标点击
    // 4. 选择对应的模式
}

void MenuScene::HandleKeyboardInput()
{
    // TODO: 处理键盘输入
    // 1. 上下键切换选项
    // 2. Enter键确认选择
    // 3. ESC键退出
}

bool MenuScene::IsMouseOver(const MenuItem &item, int mouseX, int mouseY)
{
    // TODO: 判断鼠标是否在菜单项上
    return mouseX >= item.x && mouseX <= item.x + item.width &&
           mouseY >= item.y && mouseY <= item.y + item.height;
}

void MenuScene::DrawTitle()
{
    // TODO: 绘制游戏标题
    // settextstyle(60, 0, L"微软雅黑");
    // settextcolor(YELLOW);
    // outtextxy(250, 50, L"贪吃蛇游戏");
}

void MenuScene::DrawInstructions()
{
    // TODO: 绘制操作说明
    // settextstyle(20, 0, L"微软雅黑");
    // settextcolor(WHITE);
    // outtextxy(50, 500, L"使用鼠标点击或键盘方向键选择模式");
}
