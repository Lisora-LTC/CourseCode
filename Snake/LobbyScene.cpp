#include <winsock2.h>
#include "LobbyScene.h"
#include <windows.h>

// ============== 构造与析构 ==============
LobbyScene::LobbyScene(NetworkManager *netMgr, uint32_t roomId, const std::wstring &roomName,
                       const std::wstring &playerName, bool isHost, bool manageWindow,
                       bool isReturningFromGame)
    : networkMgr(netMgr), roomId(roomId), roomName(roomName), isHost(isHost),
      sceneRunning(false), manageWindow(manageWindow),
      remotePlayerConnected(false), localReady(false), remoteReady(false),
      gameStarting(false), shouldExitLobby(false), shouldStartGame(false),
      protectionTimer(0)
{
    // 初始化本地玩家信息
    wcsncpy_s(localPlayer.nickname, 40, playerName.c_str(), 39);
    localPlayer.avatarColor = isHost ? COLOR_P1 : COLOR_P2;
    localPlayer.isHost = isHost;

    // 【修改】如果是房主，默认状态就是 READY
    if (isHost)
    {
        localPlayer.status = PLAYER_READY;
        localReady = true;
    }
    else
    {
        // P2 每次进入大厅都需要重新准备
        localPlayer.status = PLAYER_NOT_READY;
        localReady = false;
    }

    // 初始化远程玩家信息（等待连接或等待准备）
    if (isHost)
    {
        // 房主视角：对手可能还在（游戏结束返回）或刚离开
        remotePlayer.status = PLAYER_NOT_READY; // 假设对手需要重新准备
    }
    else
    {
        // P2 视角：房主一直在，默认准备好
        remotePlayer.status = PLAYER_READY;
    }
    remotePlayer.avatarColor = isHost ? COLOR_P2 : COLOR_P1;
    remotePlayer.isHost = !isHost;

    InitUI();

    // 注意：P2 的 HELLO 包发送已移到 Show() 方法中，在 ResetForNewRound() 之后
    // 这样可以避免消息被立即清空
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
    // 1. 先重置状态（清除之前的消息队列，防止读取旧数据）
    ResetForNewRound();

    // 2. 初始化图形（如果管理窗口）
    if (manageWindow)
    {
        initgraph(1920, 1080);
    }

    setbkcolor(COLOR_BG);
    cleardevice();

    // 3. 【核心修复】P2 进入时发送 Hello，但不要立即发送状态包
    // 依赖 UpdateLobbyState 中的自动重发机制来确保连接
    if (!isHost && networkMgr)
    {
        // 首次尝试发送 Hello
        HelloPacket hello;
        hello.uid = networkMgr->GetMyUID();

        std::wstring playerName(localPlayer.nickname);
        WideCharToMultiByte(CP_UTF8, 0, playerName.c_str(), -1, hello.name, 40, NULL, NULL);

        networkMgr->SendBinaryMessage((char *)&hello, sizeof(HelloPacket));
    }

    sceneRunning = true;

    while (sceneRunning)
    {
        if (inputMgr.IsWindowClosed())
        {
            sceneRunning = false;
            shouldStartGame = false;
            break;
        }

        inputMgr.Update();
        UpdateLobbyState(); // 核心逻辑都在这里
        UpdatePlayerCards();
        UpdateButtonStates();
        HandleInput();
        Render();
        Sleep(16);
    }

    return shouldStartGame;
}

void LobbyScene::ResetForNewRound()
{
    // 0. 【核心】先清洗 NetworkManager 的底层脏数据
    if (networkMgr)
    {
        networkMgr->ResetRoomState();
    }

    // 1. 重置流程控制标志
    sceneRunning = true;     // 允许大厅运行
    shouldStartGame = false; // 游戏没开始
    gameStarting = false;    // 动画停止
    shouldExitLobby = false; // 不退出大厅

    // 【核心修复】设置保护计时器，防止立即触发开始游戏
    protectionTimer = 30; // 给他 30 帧 (0.5秒) 的冷却时间

    // 2. 重置玩家准备状态
    if (isHost)
    {
        // 房主默认准备
        localReady = true;
        localPlayer.status = PLAYER_READY;

        // 【修复】不要把对手设为离线，假设他还连着（防止闪烁）
        // 如果对手已连接，只重置准备状态
        if (remotePlayerConnected)
        {
            remotePlayer.status = PLAYER_NOT_READY;
            remoteReady = false;
        }
    }
    else
    {
        // P2 需要重新准备
        localReady = false;
        localPlayer.status = PLAYER_NOT_READY;

        // 【修复】不要把房主设为离线，假设他还在（防止闪烁）
        // remotePlayer.status = PLAYER_NOT_READY; // 注释掉，保持上次状态

        // 发送 NOT_READY 消息告诉房主
        if (networkMgr)
        {
            GameCommand cmd;
            cmd.cmdType = CMD_NOT_READY;
            cmd.data = 0;
            cmd.uid = networkMgr->GetMyUID();
            networkMgr->SendBinaryMessage((char *)&cmd, sizeof(GameCommand));
        }
    }

    // 3. 更新 UI
    if (!isHost && btnReady)
    {
        btnReady->SetText(L"准备");
        btnReady->SetColors(COLOR_GREEN, RGB(85, 175, 85), RGB(56, 142, 60));
    }

    if (isHost && btnStart)
    {
        btnStart->SetEnabled(false); // 禁用开始按钮，等待 P2 准备
    }

    // 4. 【核心修复】网络状态清洗
    if (networkMgr)
    {
        // 清空网络接收状态，防止读取到上一局的残留消息
        NetworkManager::ReceivedLobbyState dummyState;
        networkMgr->GetReceivedLobbyState(dummyState); // 读取并丢弃

        if (isHost)
        {
            // 【核心】房主发送一个"重置/等待"广播，覆盖掉之前的"Start"信号
            // 通过心跳广播自然同步状态（bothReady=false）
            // 下次 UpdateLobbyState 时会自动发送当前状态
        }
    }
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

    // 处理底层消息接收（将二进制包转为内部状态）
    networkMgr->ProcessRoomMessages();

    // 【房主心跳广播】如果是房主，每 0.5 秒广播一次房间状态
    if (isHost)
    {
        static ULONGLONG lastSyncTime = 0;
        ULONGLONG now = GetTickCount64();

        if (now - lastSyncTime > 500) // 每 500ms 广播一次
        {
            lastSyncTime = now;

            // 组装状态包
            LobbyStatePacket pkg;
            pkg.uid = networkMgr->GetMyUID();

            // P1（房主）信息
            pkg.p1_exist = true;
            pkg.p1_ready = (localPlayer.status == PLAYER_READY);
            // 将 wstring 转换为 UTF-8
            WideCharToMultiByte(CP_UTF8, 0, localPlayer.nickname, -1, pkg.p1_name, 40, NULL, NULL);

            // P2（客机）信息
            pkg.p2_exist = remotePlayerConnected;
            pkg.p2_ready = (remotePlayer.status == PLAYER_READY);
            if (remotePlayerConnected)
            {
                WideCharToMultiByte(CP_UTF8, 0, remotePlayer.nickname, -1, pkg.p2_name, 40, NULL, NULL);
            }

            // 发送广播
            networkMgr->SendBinaryMessage((char *)&pkg, sizeof(LobbyStatePacket));
        }
    }

    // 【统一使用LobbyStatePacket】从网络接收大厅状态更新
    NetworkManager::ReceivedLobbyState lobbyState;
    if (networkMgr->GetReceivedLobbyState(lobbyState))
    {
        // 处理LOBBY_SYNC包（P2接收P1的心跳广播）
        if (!isHost && lobbyState.receivedLobbySync)
        {
            // 【核心修复】P2侧：收到P1的广播，说明P1在线
            remotePlayerConnected = true;

            // 更新P1（房主）信息
            wchar_t p1Name[40];
            MultiByteToWideChar(CP_UTF8, 0, lobbyState.p1Name, -1, p1Name, 40);
            wcsncpy_s(remotePlayer.nickname, 40, p1Name, 39);
            remotePlayer.status = lobbyState.p1Ready ? PLAYER_READY : PLAYER_NOT_READY;

            // 【修复点2】检查房主是否看到了我
            if (!lobbyState.p2Exist)
            {
                // 房主还没看到我！Hello包可能丢了，需要重发
                static ULONGLONG lastRetryTime = 0;
                ULONGLONG now = GetTickCount64();
                if (now - lastRetryTime > 1000) // 每1秒重试一次
                {
                    lastRetryTime = now;
                    HelloPacket hello;
                    hello.uid = networkMgr->GetMyUID();
                    std::wstring playerName(localPlayer.nickname);
                    WideCharToMultiByte(CP_UTF8, 0, playerName.c_str(), -1, hello.name, 40, NULL, NULL);
                    networkMgr->SendBinaryMessage((char *)&hello, sizeof(HelloPacket));
                }
            }
            else
            {
                // 房主看到我了，同步我的状态（以房主为准）
                localPlayer.status = lobbyState.p2Ready ? PLAYER_READY : PLAYER_NOT_READY;
                localReady = (localPlayer.status == PLAYER_READY);
            }
        }

        // 【处理游戏开始消息】P2接收
        if (lobbyState.gameStarted && !isHost)
        {
            shouldStartGame = true;
            sceneRunning = false;
            return;
        }

        // 【处理退出消息】
        if (lobbyState.exitReceived)
        {
            if (isHost)
            {
                // === 我是房主，P2 退出了 ===
                remotePlayerConnected = false;
                remotePlayer.status = PLAYER_OFFLINE;

                // 按钮变灰
                if (btnStart)
                    btnStart->SetEnabled(false);

                // UI 重置为“等待连接”
                remotePlayerCard->SetWaiting(true);

                // 发个提示
                MessageBoxW(GetHWnd(), L"玩家 P2 已退出房间", L"提示", MB_OK | MB_ICONINFORMATION);
            }
            else
            {
                // 我是客机，房主退出了
                MessageBoxW(GetHWnd(), L"房主已解散房间", L"提示", MB_OK | MB_ICONWARNING);
                shouldExitLobby = true;
                shouldStartGame = false;
                sceneRunning = false;
                return;
            }
        }

        // 【房主侧逻辑】处理P2连接通知
        if (isHost && lobbyState.p2Connected)
        {
            remotePlayerConnected = true;
            wcsncpy_s(remotePlayer.nickname, 40, lobbyState.p2Name, 39);
            // 新连入的玩家默认未准备
            if (remotePlayer.status == PLAYER_OFFLINE)
            {
                remotePlayer.status = PLAYER_NOT_READY;
            }
        }

        // 【房主侧逻辑】更新P2准备状态
        if (isHost && remotePlayerConnected)
        {
            remotePlayer.status = lobbyState.p2Ready ? PLAYER_READY : PLAYER_NOT_READY;
        }
    }

    if (protectionTimer > 0)
        protectionTimer--;
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
    // 房主的"开始游戏"按钮
    if (isHost && btnStart)
    {
        // 【修改】逻辑改为：只要 P2 连进来了且 P2 准备好了，就能开始
        // 房主自己默认就是 Ready 的，不需要判断 localPlayer.status
        bool canStart = remotePlayerConnected && (remotePlayer.status == PLAYER_READY);
        btnStart->SetEnabled(canStart);
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
            // 发送开始游戏消息
            networkMgr->SendStartGameCommand();
            shouldStartGame = true;
            sceneRunning = false;
            return;
        }

        // 非房主的准备按钮
        if (!isHost && btnReady && btnReady->IsMouseOver(mouseX, mouseY))
        {
            localReady = !localReady;
            localPlayer.status = localReady ? PLAYER_READY : PLAYER_NOT_READY;

            // 【漏洞1修复】：发送准备状态给房主
            GameCommand cmd;
            cmd.cmdType = localReady ? CMD_READY : CMD_NOT_READY;
            cmd.data = localReady ? 1 : 0;
            cmd.uid = networkMgr->GetMyUID();

            // 发送出去！
            networkMgr->SendBinaryMessage((char *)&cmd, sizeof(GameCommand));

            return;
        }

        // 退出按钮
        if (btnExit->IsMouseOver(mouseX, mouseY))
        {
            // 确认退出
            int result = MessageBoxW(GetHWnd(), L"确定要退出房间吗？", L"确认", MB_YESNO | MB_ICONQUESTION);
            if (result == IDYES)
            {
                // 【新增】发送退出通知包
                if (networkMgr)
                {
                    ExitPacket exitPkg;
                    exitPkg.uid = networkMgr->GetMyUID();
                    networkMgr->SendBinaryMessage((char *)&exitPkg, sizeof(ExitPacket));
                }

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
