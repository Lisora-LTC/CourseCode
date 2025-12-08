#pragma once
#include "Common.h"
#include "InputManager.h"
#include "UIComponent.h"
#include <graphics.h>
#include <string>
#include <memory>
#include <vector>

// ============== 联机模式选择场景 ==============
// 用户选择"创建房间"或"加入房间"

// 用户操作类型枚举
enum NetworkModeAction
{
    NM_CREATE_ROOM, // 创建房间
    NM_JOIN_ROOM,   // 加入房间
    NM_BACK,        // 返回主菜单
    NM_NONE         // 无操作（场景仍在运行）
};

class NetworkModeScene
{
private:
    InputManager inputMgr;
    bool sceneRunning;
    bool manageWindow;
    NetworkModeAction selectedAction;

    // UI按钮
    std::unique_ptr<UIButton> btnCreateRoom;
    std::unique_ptr<UIButton> btnJoinRoom;
    std::unique_ptr<UIButton> btnBack;

    // 颜色定义
    const COLORREF COLOR_BG = RGB(249, 247, 247);     // #F9F7F7 米白
    const COLORREF COLOR_BORDER = RGB(219, 226, 239); // #DBE2EF 灰蓝
    const COLORREF COLOR_ACCENT = RGB(63, 114, 175);  // #3F72AF 亮蓝
    const COLORREF COLOR_DARK = RGB(17, 45, 78);      // #112D4E 深藏青
    const COLORREF COLOR_HOVER = RGB(85, 132, 188);   // #5584BC 悬停亮蓝

public:
    // ============== 构造与析构 ==============
    NetworkModeScene(bool manageWindow = false);
    ~NetworkModeScene();

    // ============== 主方法 ==============
    /**
     * @brief 显示场景并等待用户选择
     * @return 用户选择的操作类型
     */
    NetworkModeAction Show();

private:
    // ============== 初始化 ==============
    void InitUI();

    // ============== 渲染方法 ==============
    void Render();
    void DrawTitle();
    void DrawButtons();

    // ============== 输入处理 ==============
    void HandleInput();
};
