#pragma once
#include "Common.h"
#include "InputManager.h"
#include "NetworkManager.h"
#include <graphics.h>
#include <string>
#include <vector>

// ============== 房间列表场景 ==============
// 显示在线房间列表，用户可以选择加入某个房间

class RoomListScene
{
private:
    InputManager inputMgr;
    NetworkManager *networkMgr; // 网络管理器（外部传入）
    bool sceneRunning;
    bool manageWindow;

    std::vector<RoomInfo> rooms; // 房间列表
    int selectedRoomIndex;       // 选中的房间索引（-1表示未选中）
    bool joinSuccessful;         // 是否成功加入房间
    uint32_t joinedRoomId;       // 加入的房间ID

    // 颜色定义（复用HistoryScene的配色）
    const COLORREF COLOR_BG = RGB(249, 247, 247);     // #F9F7F7 米白
    const COLORREF COLOR_BORDER = RGB(219, 226, 239); // #DBE2EF 灰蓝
    const COLORREF COLOR_ACCENT = RGB(63, 114, 175);  // #3F72AF 亮蓝
    const COLORREF COLOR_DARK = RGB(17, 45, 78);      // #112D4E 深藏青
    const COLORREF COLOR_WHITE = RGB(249, 247, 247);  // 白色
    const COLORREF COLOR_GREEN = RGB(76, 175, 80);    // 绿色（等待中）
    const COLORREF COLOR_RED = RGB(244, 67, 54);      // 红色（游戏中）
    const COLORREF COLOR_GRAY = RGB(158, 158, 158);   // 灰色（满员）

public:
    // ============== 构造与析构 ==============
    RoomListScene(NetworkManager *netMgr, bool manageWindow = false);
    ~RoomListScene();

    // ============== 主方法 ==============
    /**
     * @brief 显示房间列表场景
     * @return 是否成功加入房间
     */
    bool Show();

    /**
     * @brief 获取加入的房间ID
     */
    uint32_t GetJoinedRoomId() const { return joinedRoomId; }

private:
    // ============== 数据加载 ==============
    void LoadRooms();
    void RefreshRoomList();

    // ============== 渲染方法 ==============
    void Render();
    void DrawHeader();
    void DrawTableHeader();
    void DrawRoomList();
    void DrawSingleRoom(const RoomInfo &room, int index, int x, int y, int width, int height);
    void DrawStatusDot(int x, int y, RoomStatus status);
    void DrawFooter();

    // ============== 输入处理 ==============
    void HandleInput();
    bool IsMouseInRect(int mouseX, int mouseY, int x, int y, int width, int height);

    // ============== 工具方法 ==============
    void DrawRoundRect(int x, int y, int width, int height, int radius, COLORREF fillColor, COLORREF borderColor);
    std::wstring RoomStatusToString(RoomStatus status);
};
