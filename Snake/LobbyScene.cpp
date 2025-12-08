#include <winsock2.h>
#include "LobbyScene.h"
#include <windows.h>

// ============== 构造与析构 ==============
LobbyScene::LobbyScene(NetworkManager *netMgr, uint32_t roomId, const std::wstring &roomName,
                       const std::wstring &playerName, bool isHost, bool manageWindow)
    : networkMgr(netMgr), roomId(roomId), roomName(roomName), isHost(isHost),
      sceneRunning(false), manageWindow(manageWindow),
      remotePlayerConnected(false), localReady(false), remoteReady(false),
      gameStarting(false), shouldExitLobby(false), shouldStartGame(false)
{
    // 初始化本地玩家信息
    wcsncpy_s(localPlayer.nickname, 40, playerName.c_str(), 39);
    localPlayer.status = PLAYER_NOT_READY;
    localPlayer.avatarColor = isHost ? COLOR_P1 : COLOR_P2;
    localPlayer.isHost = isHost;

    // 初始化远程玩家信息（等待连接）
    remotePlayer.status = PLAYER_OFFLINE;
    remotePlayer.avatarColor = isHost ? COLOR_P2 : COLOR_P1;
    remotePlayer.isHost = !isHost;

    InitUI();
}

LobbyScene::~LobbyScene()
{
    if (manageWindow)
    {
        closegraph();
    }
}

// ============== 主方法 ==============
bool LobbyScene::Show()
{
    if (manageWindow)
    {
        initgraph(1920, 1080);
    }

    setbkcolor(COLOR_BG);
    cleardevice();

    sceneRunning = true;

    while (sceneRunning)
    {
        if (inputMgr.IsWindowClosed())
        {
            // 窗口关闭时正常退出
            sceneRunning = false;
            shouldStartGame = false;
            break;
        }

        inputMgr.Update();
        UpdateLobbyState();
        UpdatePlayerCards();
        UpdateButtonStates();
        HandleInput();
        Render();
        Sleep(16); // ~60 FPS
    }

    return shouldStartGame;
}

// ============== 初始化 ==============
void LobbyScene::InitUI()
{
    // 计算卡片位置（左右两侧）
    int cardWidth = 600;
    int cardHeight = 500;
    int cardY = 250;
    int leftCardX = 200;   // 左侧卡片X坐标
    int rightCardX = 1120; // 右侧卡片X坐标

    // 创建左侧玩家卡片（本地玩家 - 总是你）
    localPlayerCard = std::make_unique<UIPlayerCard>(leftCardX, cardY, cardWidth, cardHeight);
    localPlayerCard->SetColors(localPlayer.avatarColor, COLOR_ACCENT, RGB(249, 247, 247));
    localPlayerCard->SetWaiting(false);

    // 创建右侧玩家卡片（远程玩家 - 等待加入或显示对手）
    remotePlayerCard = std::make_unique<UIPlayerCard>(rightCardX, cardY, cardWidth, cardHeight);
    remotePlayerCard->SetColors(remotePlayer.avatarColor, COLOR_BORDER, RGB(235, 235, 235));
    remotePlayerCard->SetWaiting(true); // 初始为等待状态

    // 创建加载动画（位于右侧卡片中心）
    int animX = rightCardX + cardWidth / 2;
    int animY = cardY + cardHeight / 2 - 50;
    loadingAnim = std::make_unique<UILoadingAnimation>(animX, animY, 40, ANIM_DOTS_JUMPING);
    loadingAnim->SetColor(COLOR_ACCENT);

    // 创建按钮
    int btnWidth = 200;
    int btnHeight = 60;
    int btnY = 850;
    int btnSpacing = 250;
    int centerX = 960;

    // 准备按钮（非房主）/ 开始游戏按钮（房主）
    if (isHost)
    {
        btnStart = std::make_unique<UIButton>(
            centerX - btnWidth - btnSpacing / 2, btnY, btnWidth, btnHeight,
            L"开始游戏", BUTTON_CAPSULE);
        btnStart->SetColors(RGB(76, 175, 80), RGB(56, 142, 60), RGB(27, 94, 32));
        btnStart->SetTextColor(RGB(255, 255, 255), RGB(255, 255, 255));
        btnStart->SetFontSize(36);
        btnStart->SetEnabled(false); // 初始禁用，等双方都准备好
    }
    else
    {
        btnReady = std::make_unique<UIButton>(
            centerX - btnWidth - btnSpacing / 2, btnY, btnWidth, btnHeight,
            L"准备", BUTTON_CAPSULE);
        btnReady->SetColors(RGB(219, 226, 239), RGB(85, 132, 188), RGB(63, 114, 175));
        btnReady->SetTextColor(RGB(17, 45, 78), RGB(255, 255, 255));
        btnReady->SetFontSize(36);
    }

    // 退出按钮
    btnExit = std::make_unique<UIButton>(
        centerX + btnSpacing / 2, btnY, btnWidth, btnHeight,
        L"退出房间", BUTTON_CAPSULE);
    btnExit->SetColors(RGB(219, 226, 239), RGB(85, 132, 188), RGB(63, 114, 175));
    btnExit->SetTextColor(RGB(17, 45, 78), RGB(255, 255, 255));
    btnExit->SetFontSize(36);
}

// ============== 更新逻辑 ==============
void LobbyScene::UpdateLobbyState()
{
    if (!networkMgr)
        return;

    // 处理接收到的房间消息（更新准备状态）
    networkMgr->ProcessRoomMessages();

    // 从网络接收大厅状态更新
    LobbyUpdatePacket packet;
    if (networkMgr->ReceiveLobbyUpdate(packet))
    {
        // 更新远程玩家信息
        if (packet.player2Connected)
        {
            remotePlayerConnected = true;
            remotePlayer = isHost ? packet.player2 : packet.player1;
        }
        else
        {
            remotePlayerConnected = false;
        }

        // 检查是否可以开始游戏
        if (packet.bothReady && isHost && btnStart)
        {
            btnStart->SetEnabled(true);
        }

        // 检查游戏是否开始
        if (packet.messageType == 'S')
        {
            gameStarting = true;
            shouldStartGame = true;
            sceneRunning = false;
        }
    }

    // 定期查询房间玩家状态（每10帧查询一次，约0.17秒）
    static int queryCounter = 0;
    if (++queryCounter >= 10)
    {
        queryCounter = 0;

        std::vector<LobbyPlayerInfo> players;
        if (networkMgr->GetCurrentRoomPlayers(players))
        {
            if (players.size() >= 2)
            {
                remotePlayerConnected = true;
                // 更新远程玩家信息（包括准备状态）
                remotePlayer = players[isHost ? 1 : 0];

                // 更新本地玩家的准备状态（从服务器同步）
                localPlayer = players[isHost ? 0 : 1];
            }
            else
            {
                remotePlayerConnected = false;
            }
        }
    }
}

void LobbyScene::UpdatePlayerCards()
{
    // 更新本地玩家卡片
    std::wstring localStatusText = PlayerStatusToString(localPlayer.status);
    std::wstring localNickname(localPlayer.nickname);
    localPlayerCard->SetPlayerInfo(
        localNickname,
        localStatusText,
        localPlayer.avatarColor);

    // 更新远程玩家卡片
    if (remotePlayerConnected)
    {
        std::wstring remoteStatusText = PlayerStatusToString(remotePlayer.status);
        std::wstring remoteNickname(remotePlayer.nickname);
        remotePlayerCard->SetWaiting(false);
        remotePlayerCard->SetPlayerInfo(
            remoteNickname,
            remoteStatusText,
            remotePlayer.avatarColor);
    }
    else
    {
        remotePlayerCard->SetWaiting(true);
    }
}

void LobbyScene::UpdateButtonStates()
{
    // 房主的"开始游戏"按钮：只有双方都准备好才能点击
    if (isHost && btnStart)
    {
        bool bothReady = (localPlayer.status == PLAYER_READY) &&
                         (remotePlayer.status == PLAYER_READY) &&
                         remotePlayerConnected;
        btnStart->SetEnabled(bothReady);
    }

    // 非房主的"准备"按钮：切换文字
    if (!isHost && btnReady)
    {
        if (localReady)
        {
            btnReady->SetText(L"取消准备");
            btnReady->SetColors(RGB(158, 158, 158), COLOR_HOVER, COLOR_DARK);
        }
        else
        {
            btnReady->SetText(L"准备");
            btnReady->SetColors(COLOR_ACCENT, COLOR_HOVER, COLOR_DARK);
        }
    }
}

// ============== 渲染方法 ==============
void LobbyScene::Render()
{
    BeginBatchDraw();

    setbkcolor(COLOR_BG);
    cleardevice();

    DrawRoomInfo();
    DrawPlayerCards();
    DrawButtons();

    EndBatchDraw();
}

void LobbyScene::DrawRoomInfo()
{
    // 绘制房间信息栏
    settextcolor(COLOR_DARK);
    settextstyle(40, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);

    // 房间号
    wchar_t roomIdStr[100];
    swprintf_s(roomIdStr, L"房间号: %u", roomId);
    int idWidth = textwidth(roomIdStr);
    outtextxy(960 - idWidth / 2 - 200, 80, roomIdStr);

    // 分隔符
    outtextxy(960 - 10, 80, L"|");

    // 房间名
    wchar_t roomNameStr[100];
    swprintf_s(roomNameStr, L"房间名: %s", roomName.c_str());
    int nameWidth = textwidth(roomNameStr);
    outtextxy(960 - nameWidth / 2 + 200, 80, roomNameStr);
}

void LobbyScene::DrawPlayerCards()
{
    // 绘制左侧卡片（本地玩家）
    localPlayerCard->Draw();

    // 绘制右侧卡片（远程玩家或等待状态）
    remotePlayerCard->Draw();

    // 如果远程玩家未连接，绘制加载动画
    if (!remotePlayerConnected)
    {
        loadingAnim->Draw();
    }
}

void LobbyScene::DrawButtons()
{
    // 更新按钮状态
    int mouseX, mouseY;
    inputMgr.GetMousePosition(mouseX, mouseY);
    bool mousePressed = GetAsyncKeyState(VK_LBUTTON) & 0x8000;

    // 绘制按钮
    if (isHost && btnStart)
    {
        btnStart->UpdateState(mouseX, mouseY, mousePressed);
        btnStart->Draw();
    }
    else if (!isHost && btnReady)
    {
        btnReady->UpdateState(mouseX, mouseY, mousePressed);
        btnReady->Draw();
    }

    btnExit->UpdateState(mouseX, mouseY, mousePressed);
    btnExit->Draw();
}

// ============== 输入处理 ==============
void LobbyScene::HandleInput()
{
    if (inputMgr.IsLeftButtonJustPressed())
    {
        int mouseX, mouseY;
        inputMgr.GetMousePosition(mouseX, mouseY);

        // 房主的开始游戏按钮
        if (isHost && btnStart && btnStart->IsMouseOver(mouseX, mouseY) && btnStart->IsEnabled())
        {
            // TODO: 发送开始游戏消息
            // networkMgr->SendStartGameCommand();
            shouldStartGame = true;
            sceneRunning = false;
            return;
        }

        // 非房主的准备按钮
        if (!isHost && btnReady && btnReady->IsMouseOver(mouseX, mouseY))
        {
            localReady = !localReady;
            localPlayer.status = localReady ? PLAYER_READY : PLAYER_NOT_READY;

            // TODO: 发送准备状态到服务器
            // networkMgr->SetPlayerReady(localReady);
            // LobbyUpdatePacket packet;
            // packet.player1 = localPlayer;
            // networkMgr->SendLobbyUpdate(packet);
            return;
        }

        // 退出按钮
        if (btnExit->IsMouseOver(mouseX, mouseY))
        {
            // 确认退出
            int result = MessageBoxW(GetHWnd(), L"确定要退出房间吗？", L"确认", MB_YESNO | MB_ICONQUESTION);
            if (result == IDYES)
            {
                // TODO: 通知服务器离开房间
                // networkMgr->LeaveRoom();
                shouldExitLobby = true;
                shouldStartGame = false;
                sceneRunning = false;
            }
            return;
        }
    }

    // ESC键退出
    if (inputMgr.IsKeyJustPressed(VK_ESCAPE))
    {
        int result = MessageBoxW(GetHWnd(), L"确定要退出房间吗？", L"确认", MB_YESNO | MB_ICONQUESTION);
        if (result == IDYES)
        {
            shouldExitLobby = true;
            shouldStartGame = false;
            sceneRunning = false;
        }
    }
}

// ============== 工具方法 ==============
void LobbyScene::DrawRoundRect(int x, int y, int width, int height, int radius, COLORREF fillColor, COLORREF borderColor)
{
    setfillcolor(fillColor);
    setlinecolor(borderColor);
    setlinestyle(PS_SOLID, 2);
    fillroundrect(x, y, x + width, y + height, radius, radius);
}

std::wstring LobbyScene::PlayerStatusToString(PlayerStatus status)
{
    switch (status)
    {
    case PLAYER_READY:
        return L"已准备";
    case PLAYER_NOT_READY:
        return L"未准备";
    case PLAYER_OFFLINE:
        return L"离线";
    default:
        return L"未知";
    }
}
