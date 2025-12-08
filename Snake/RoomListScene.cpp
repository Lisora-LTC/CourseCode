#include <winsock2.h>
#include "RoomListScene.h"
#include <windows.h>
#include <sstream>

// ============== 构造与析构 ==============
RoomListScene::RoomListScene(NetworkManager *netMgr, bool manageWindow)
    : networkMgr(netMgr), sceneRunning(false), manageWindow(manageWindow),
      selectedRoomIndex(-1), joinSuccessful(false), joinedRoomId(0)
{
    LoadRooms();
}

RoomListScene::~RoomListScene()
{
    if (manageWindow)
    {
        closegraph();
    }
}

// ============== 主方法 ==============
bool RoomListScene::Show()
{
    if (manageWindow)
    {
        initgraph(1920, 1080);
    }

    setbkcolor(COLOR_BG);
    cleardevice();

    sceneRunning = true;
    joinSuccessful = false;

    while (sceneRunning)
    {
        if (inputMgr.IsWindowClosed())
        {
            // 窗口关闭时正常退出
            sceneRunning = false;
            break;
        }

        inputMgr.Update();
        HandleInput();
        Render();
        Sleep(10);
    }

    return joinSuccessful;
}

// ============== 数据加载 ==============
void RoomListScene::LoadRooms()
{
    RefreshRoomList();
}

void RoomListScene::RefreshRoomList()
{
    rooms.clear();
    if (networkMgr)
    {
        networkMgr->GetRoomList(rooms);
    }
}

// ============== 渲染方法 ==============
void RoomListScene::Render()
{
    BeginBatchDraw();

    setbkcolor(COLOR_BG);
    cleardevice();

    DrawHeader();
    DrawTableHeader();
    DrawRoomList();
    DrawFooter();

    EndBatchDraw();
}

void RoomListScene::DrawHeader()
{
    // 绘制标题
    settextcolor(COLOR_DARK);
    settextstyle(64, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);

    const wchar_t *title = L"房间列表";
    int titleWidth = textwidth(title);
    outtextxy(960 - titleWidth / 2, 60, title);

    // 绘制刷新按钮（小圆圈图标）
    int refreshX = 1750;
    int refreshY = 90;
    int refreshRadius = 30;

    setlinecolor(COLOR_ACCENT);
    setlinestyle(PS_SOLID, 3);
    setfillcolor(COLOR_WHITE);
    fillcircle(refreshX, refreshY, refreshRadius);

    // 绘制刷新图标（简化为圆弧箭头）
    setlinecolor(COLOR_ACCENT);
    arc(refreshX - 15, refreshY - 15, refreshX + 15, refreshY + 15, 0.5, 5.5);

    // 箭头
    line(refreshX + 12, refreshY - 8, refreshX + 15, refreshY - 15);
    line(refreshX + 12, refreshY - 8, refreshX + 5, refreshY - 10);

    // 检测刷新按钮点击（在HandleInput中处理）
}

void RoomListScene::DrawTableHeader()
{
    // 表头位置
    int headerY = 180;
    int headerHeight = 50;
    int headerX = 200;
    int headerWidth = 1520;

    // 绘制表头背景
    setfillcolor(COLOR_BORDER);
    setlinecolor(COLOR_BORDER);
    fillrectangle(headerX, headerY, headerX + headerWidth, headerY + headerHeight);

    // 表头文字
    settextcolor(COLOR_DARK);
    settextstyle(32, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);

    // 列宽分配
    int col1Width = 200; // 房间ID
    int col2Width = 400; // 房间名
    int col3Width = 300; // 房主
    int col4Width = 300; // 状态
    int col5Width = 320; // 人数

    int currentX = headerX + 20;

    outtextxy(currentX, headerY + 10, L"房间ID");
    currentX += col1Width;

    outtextxy(currentX, headerY + 10, L"房间名");
    currentX += col2Width;

    outtextxy(currentX, headerY + 10, L"房主");
    currentX += col3Width;

    outtextxy(currentX, headerY + 10, L"状态");
    currentX += col4Width;

    outtextxy(currentX, headerY + 10, L"人数");
}

void RoomListScene::DrawRoomList()
{
    int startY = 250;
    int cardHeight = 80;
    int cardSpacing = 20;
    int cardX = 200;
    int cardWidth = 1520;

    for (size_t i = 0; i < rooms.size(); i++)
    {
        int y = startY + static_cast<int>(i) * (cardHeight + cardSpacing);
        DrawSingleRoom(rooms[i], static_cast<int>(i), cardX, y, cardWidth, cardHeight);
    }

    // 如果没有房间
    if (rooms.empty())
    {
        settextcolor(RGB(150, 150, 150));
        settextstyle(40, 0, L"微软雅黑");
        const wchar_t *msg = L"暂无在线房间";
        int msgWidth = textwidth(msg);
        outtextxy(960 - msgWidth / 2, 400, msg);
    }
}

void RoomListScene::DrawSingleRoom(const RoomInfo &room, int index, int x, int y, int width, int height)
{
    // 判断是否满员或游戏中
    bool isFull = (room.currentPlayers >= room.maxPlayers);
    bool isPlaying = (room.status == ROOM_PLAYING);
    bool isDisabled = isFull || isPlaying;

    // 卡片背景色
    COLORREF bgColor = isDisabled ? RGB(230, 230, 230) : COLOR_WHITE;
    COLORREF cardBorderColor = (selectedRoomIndex == index) ? COLOR_ACCENT : COLOR_BORDER;

    // 绘制卡片
    DrawRoundRect(x, y, width, height, 10, bgColor, cardBorderColor);

    // 列宽分配（与表头一致）
    int col1Width = 200;
    int col2Width = 400;
    int col3Width = 300;
    int col4Width = 300;
    int col5Width = 320;

    int currentX = x + 20;
    int textY = y + (height - 32) / 2;

    // 设置文字颜色
    settextcolor(isDisabled ? COLOR_GRAY : COLOR_DARK);
    settextstyle(28, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);

    // 房间ID
    wchar_t roomIdStr[32];
    swprintf_s(roomIdStr, 32, L"%u", room.roomId);
    outtextxy(currentX, textY, roomIdStr);
    currentX += col1Width;

    // 房间名
    outtextxy(currentX, textY, room.roomName);
    currentX += col2Width;

    // 房主
    outtextxy(currentX, textY, room.hostName);
    currentX += col3Width;

    // 状态（带状态点）
    DrawStatusDot(currentX, y + height / 2, room.status);
    std::wstring statusStr = RoomStatusToString(room.status);
    outtextxy(currentX + 30, textY, statusStr.c_str());
    currentX += col4Width;

    // 人数
    wchar_t playersStr[32];
    swprintf_s(playersStr, 32, L"%d / %d", room.currentPlayers, room.maxPlayers);
    settextcolor(isFull ? COLOR_RED : COLOR_DARK);
    outtextxy(currentX, textY, playersStr);
}

void RoomListScene::DrawStatusDot(int x, int y, RoomStatus status)
{
    COLORREF dotColor;
    switch (status)
    {
    case ROOM_WAITING:
        dotColor = COLOR_GREEN;
        break;
    case ROOM_PLAYING:
        dotColor = COLOR_RED;
        break;
    case ROOM_FULL:
        dotColor = COLOR_GRAY;
        break;
    default:
        dotColor = COLOR_GRAY;
        break;
    }

    setfillcolor(dotColor);
    setlinecolor(dotColor);
    fillcircle(x, y, 8);
}

void RoomListScene::DrawFooter()
{
    // 绘制返回按钮（胶囊形）
    int btnX = 860;
    int btnY = 950;
    int btnWidth = 200;
    int btnHeight = 60;
    int radius = btnHeight / 2;

    // 绘制胶囊形按钮
    setfillcolor(COLOR_BORDER);
    setlinecolor(COLOR_BORDER);
    // 左半圆
    solidcircle(btnX + radius, btnY + radius, radius);
    // 右半圆
    solidcircle(btnX + btnWidth - radius, btnY + radius, radius);
    // 中间矩形
    solidrectangle(btnX + radius, btnY, btnX + btnWidth - radius, btnY + btnHeight);

    settextcolor(COLOR_DARK);
    settextstyle(32, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);
    const wchar_t *btnText = L"返回";
    int textWidth = textwidth(btnText);
    int textHeight = textheight(btnText);
    outtextxy(btnX + (btnWidth - textWidth) / 2, btnY + (btnHeight - textHeight) / 2, btnText);
}

// ============== 输入处理 ==============
void RoomListScene::HandleInput()
{
    if (inputMgr.IsLeftButtonJustPressed())
    {
        int mouseX, mouseY;
        inputMgr.GetMousePosition(mouseX, mouseY);

        // 检测刷新按钮点击
        int refreshX = 1750;
        int refreshY = 90;
        int refreshRadius = 30;
        if (Utils::DistanceSquared(Point(mouseX, mouseY), Point(refreshX, refreshY)) <= refreshRadius * refreshRadius)
        {
            RefreshRoomList();
            return;
        }

        // 检测房间卡片点击
        int startY = 250;
        int cardHeight = 80;
        int cardSpacing = 20;
        int cardX = 200;
        int cardWidth = 1520;

        for (size_t i = 0; i < rooms.size(); i++)
        {
            int y = startY + (int)i * (cardHeight + cardSpacing);
            if (IsMouseInRect(mouseX, mouseY, cardX, y, cardWidth, cardHeight))
            {
                // 检查是否可以加入
                if (rooms[i].currentPlayers >= rooms[i].maxPlayers)
                {
                    // 满员，弹出提示
                    MessageBoxW(GetHWnd(), L"该房间已满员！", L"提示", MB_OK | MB_ICONINFORMATION);
                    return;
                }
                if (rooms[i].status == ROOM_PLAYING)
                {
                    // 游戏中，弹出提示
                    MessageBoxW(GetHWnd(), L"该房间正在游戏中！", L"提示", MB_OK | MB_ICONINFORMATION);
                    return;
                }

                // 尝试加入房间
                if (networkMgr && networkMgr->JoinRoom(rooms[i].roomId))
                {
                    joinSuccessful = true;
                    joinedRoomId = rooms[i].roomId;
                    sceneRunning = false;
                }
                else
                {
                    MessageBoxW(GetHWnd(), L"加入房间失败！", L"错误", MB_OK | MB_ICONERROR);
                }
                return;
            }
        }

        // 检测返回按钮点击
        if (IsMouseInRect(mouseX, mouseY, 860, 950, 200, 60))
        {
            sceneRunning = false;
        }
    }

    // ESC键返回
    if (inputMgr.IsKeyJustPressed(VK_ESCAPE))
    {
        sceneRunning = false;
    }
}

bool RoomListScene::IsMouseInRect(int mouseX, int mouseY, int x, int y, int width, int height)
{
    return mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height;
}

// ============== 工具方法 ==============
void RoomListScene::DrawRoundRect(int x, int y, int width, int height, int radius, COLORREF fillColor, COLORREF borderColor)
{
    setfillcolor(fillColor);
    setlinecolor(borderColor);
    setlinestyle(PS_SOLID, 2);
    fillroundrect(x, y, x + width, y + height, radius, radius);
}

std::wstring RoomListScene::RoomStatusToString(RoomStatus status)
{
    switch (status)
    {
    case ROOM_WAITING:
        return L"等待中";
    case ROOM_PLAYING:
        return L"游戏中";
    case ROOM_FULL:
        return L"已满员";
    default:
        return L"未知";
    }
}
