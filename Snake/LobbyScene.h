#pragma once
#include "Common.h"
#include "InputManager.h"
#include "NetworkManager.h"
#include "UIComponent.h"
#include <graphics.h>
#include <string>
#include <memory>

// ============== 备战大厅场景 ==============
// 双玩家准备界面，显示房主和客人的状态

class LobbyScene
{
private:
    InputManager inputMgr;
    NetworkManager *networkMgr; // 网络管理器（外部传入）
    bool sceneRunning;
    bool manageWindow;

    // 房间信息
    uint32_t roomId;
    std::wstring roomName;
    bool isHost; // 是否为房主

    // 玩家信息
    LobbyPlayerInfo localPlayer;  // 本地玩家（自己）
    LobbyPlayerInfo remotePlayer; // 远程玩家（对手）
    bool remotePlayerConnected;   // 对手是否已连接

    // UI 组件
    std::unique_ptr<UIPlayerCard> localPlayerCard;   // 本地玩家卡片
    std::unique_ptr<UIPlayerCard> remotePlayerCard;  // 远程玩家卡片
    std::unique_ptr<UILoadingAnimation> loadingAnim; // 加载动画
    std::unique_ptr<UIButton> btnReady;              // 准备按钮
    std::unique_ptr<UIButton> btnStart;              // 开始游戏按钮（仅房主可见）
    std::unique_ptr<UIButton> btnExit;               // 退出按钮

    // 状态标志
    bool localReady;      // 本地玩家是否准备
    bool remoteReady;     // 远程玩家是否准备
    bool gameStarting;    // 游戏是否即将开始
    bool shouldExitLobby; // 是否退出大厅
    bool shouldStartGame; // 是否开始游戏
    int protectionTimer;  // 保护计时器，防止刚进大厅就开始游戏（残留消息）

    // 颜色定义（复用统一配色）
    const COLORREF COLOR_BG = RGB(249, 247, 247);     // #F9F7F7 米白
    const COLORREF COLOR_BORDER = RGB(219, 226, 239); // #DBE2EF 灰蓝
    const COLORREF COLOR_ACCENT = RGB(63, 114, 175);  // #3F72AF 亮蓝
    const COLORREF COLOR_DARK = RGB(17, 45, 78);      // #112D4E 深藏青
    const COLORREF COLOR_WHITE = RGB(249, 247, 247);  // 白色
    const COLORREF COLOR_GREEN = RGB(76, 175, 80);    // 绿色（已准备）
    const COLORREF COLOR_HOVER = RGB(85, 132, 188);   // #5584BC 悬停亮蓝

    // P1 和 P2 的头像颜色
    const COLORREF COLOR_P1 = RGB(63, 114, 175); // 亮蓝色
    const COLORREF COLOR_P2 = RGB(244, 67, 54);  // 红色

public:
    // ============== 构造与析构 ==============
    /**
     * @brief 构造大厅场景
     * @param netMgr 网络管理器
     * @param roomId 房间ID
     * @param roomName 房间名称
     * @param playerName 本地玩家昵称
     * @param isHost 是否为房主
     * @param manageWindow 是否管理窗口生命周期
     * @param isReturningFromGame 是否从游戏返回（如果是，P2 不发送 HELLO）
     */
    LobbyScene(NetworkManager *netMgr, uint32_t roomId, const std::wstring &roomName,
               const std::wstring &playerName, bool isHost, bool manageWindow = false,
               bool isReturningFromGame = false);
    ~LobbyScene();

    // ============== 主方法 ==============
    /**
     * @brief 显示大厅场景
     * @return true 如果双方准备好并开始游戏，false 如果退出大厅
     */
    bool Show();

    /**
     * @brief 是否应该开始游戏
     */
    bool ShouldStartGame() const { return shouldStartGame; }

    /**
     * @brief 重置大厅状态，用于游戏结束后返回大厅
     */
    void ResetForNewRound();

private:
    // ============== 初始化 ==============
    void InitUI();

    // ============== 更新逻辑 ==============
    void UpdateLobbyState();   // 更新大厅状态（从网络同步）
    void UpdatePlayerCards();  // 更新玩家卡片显示
    void UpdateButtonStates(); // 更新按钮状态

    // ============== 渲染方法 ==============
    void Render();
    void DrawRoomInfo();    // 绘制顶部房间信息栏
    void DrawPlayerCards(); // 绘制左右两侧玩家卡片
    void DrawButtons();     // 绘制底部按钮

    // ============== 输入处理 ==============
    void HandleInput();

    // ============== 工具方法 ==============
    void DrawRoundRect(int x, int y, int width, int height, int radius, COLORREF fillColor, COLORREF borderColor);
    std::wstring PlayerStatusToString(PlayerStatus status);
};
