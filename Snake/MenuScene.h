#pragma once
#include "Common.h"
#include "InputManager.h"
#include "UIComponent.h"
#include <graphics.h>
#include <string>
#include <memory>

// ============== 菜单场景 ==============
// 处理进入游戏前的菜单界面
class MenuScene
{
private:
    enum MenuType
    {
        MAIN_MENU,
        SINGLEPLAYER_MENU,
        MULTIPLAYER_MENU
    };

    int selectedOption;   // 当前选中的选项
    bool menuRunning;     // 菜单是否运行中
    MenuType currentMenu; // 当前菜单类型
    bool manageWindow;    // 是否管理窗口生命周期

    InputManager inputMgr; // 统一输入管理器

    // 菜单选项数据（简化版，绘制由UIButton负责）
    struct MenuItem
    {
        std::wstring text;
        GameMode mode = SINGLE;
        bool isSubmenu = false; // 是否进入子菜单
        bool isExit = false;    // 是否退出
        bool isHistory = false; // 是否历史记录按钮
    };

    std::vector<MenuItem> menuItems;
    std::vector<std::unique_ptr<UIButton>> buttons; // UI按钮对象

public:
    // ============== 构造与析构 ==============
    /**
     * @brief 构造函数
     * @param manageWindow 是否由MenuScene管理窗口生命周期（默认false）
     */
    MenuScene(bool manageWindow = false);
    ~MenuScene();

    // ============== 主方法 ==============
    /**
     * @brief 显示菜单并返回用户选择的模式
     * @return 用户选择的游戏模式
     */
    GameMode Show();

private:
    /**
     * @brief 初始化主菜单项
     */
    void InitMainMenu();

    /**
     * @brief 初始化单人游戏子菜单
     */
    void InitSinglePlayerMenu();

    /**
     * @brief 初始化双人游戏子菜单
     */
    void InitMultiplayerMenu();

    /**
     * @brief 创建按钮（根据菜单项列表）
     */
    void CreateButtons();

    /**
     * @brief 渲染菜单
     */
    void Render();

    /**
     * @brief 处理鼠标输入
     */
    void HandleMouseInput();

    /**
     * @brief 处理键盘输入
     */
    void HandleKeyboardInput();

    /**
     * @brief 绘制标题
     */
    void DrawTitle();

    /**
     * @brief 绘制说明文字
     */
    void DrawInstructions();

    /**
     * @brief 显示历史记录场景
     */
    void ShowHistoryScene();
};
