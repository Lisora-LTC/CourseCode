#pragma once
#include "Common.h"
#include <graphics.h>
#include <string>

// ============== 菜单场景 ==============
// 处理进入游戏前的菜单界面
class MenuScene
{
private:
    int selectedOption; // 当前选中的选项
    bool menuRunning;   // 菜单是否运行中

    // 菜单选项
    struct MenuItem
    {
        std::wstring text;
        GameMode mode;
        int x, y, width, height;
    };

    std::vector<MenuItem> menuItems;

public:
    // ============== 构造与析构 ==============
    MenuScene();
    ~MenuScene();

    // ============== 主方法 ==============
    /**
     * @brief 显示菜单并返回用户选择的模式
     * @return 用户选择的游戏模式
     */
    GameMode Show();

private:
    /**
     * @brief 初始化菜单项
     */
    void InitMenuItems();

    /**
     * @brief 渲染菜单
     */
    void Render();

    /**
     * @brief 绘制菜单项
     */
    void DrawMenuItem(const MenuItem &item, bool isSelected);

    /**
     * @brief 处理鼠标输入
     */
    void HandleMouseInput();

    /**
     * @brief 处理键盘输入
     */
    void HandleKeyboardInput();

    /**
     * @brief 检测鼠标是否在某个菜单项上
     */
    bool IsMouseOver(const MenuItem &item, int mouseX, int mouseY);

    /**
     * @brief 绘制标题
     */
    void DrawTitle();

    /**
     * @brief 绘制说明文字
     */
    void DrawInstructions();
};
