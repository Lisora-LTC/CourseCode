#include "NetworkManager.h"
#include "Client.h"
#include <cstring>
#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>
#include <map>

// ============== 构造与析构 ==============
NetworkManager::NetworkManager()
    : client(nullptr), connected(false), myPlayerId(-1), frameCounter(0), serverPort(0),
      currentRoomId(0), inRoom(false), isHost(false), playerReady(false),
      myUID(0), opponentUID(0)
{
    // 创建 Client 实例
    client = new NetworkHelper::ClientHelper();

    // 生成唯一 UID
    InitUID();
}

NetworkManager::~NetworkManager()
{
    Disconnect();
    if (client)
    {
        delete client;
        client = nullptr;
    }
    // 服务器进程由 GlobalServerManager 自动清理
}

// ============== UID 管理 ==============
void NetworkManager::InitUID()
{
    // 使用时间戳 + 进程ID + 随机数生成唯一标识符
    // 这样即使在同一台电脑同时运行多个客户端，UID 也不会重复
    srand((unsigned int)time(NULL) + GetCurrentProcessId());

    // 生成一个大的随机数作为 UID
    // 使用两个 rand() 相乘，范围约在 0 到 10^9
    myUID = ((unsigned int)rand() << 16) | (unsigned int)rand();

    // 确保不为 0（0 用作未初始化标记）
    if (myUID == 0)
    {
        myUID = 1;
    }

    // 初始时对手 UID 为 0（未知）
    opponentUID = 0;
}

// ============== 连接管理 ==============
bool NetworkManager::Connect(const std::string &ip, int port, const std::string &name)
{
    if (!client)
    {
        return false;
    }

    // 如果连接本地服务器，先自动启动 serv.exe
    if (ip == "127.0.0.1" || ip == "localhost")
    {
        if (!GlobalServerManager::GetInstance().StartServer())
        {
            // 启动失败，但继续尝试连接（可能服务器已经在运行）
        }
        // 等待服务器启动
        Sleep(1000);
    }

    serverIp = ip;
    serverPort = port;
    playerName = name;

    // 调用 Client.h 的 connectToServer 方法
    // 注意：这个方法会阻塞直到连接成功或超时
    if (client->connectToServer(ip.c_str(), port, name.c_str()))
    {
        connected = true;
        return true;
    }

    connected = false;
    return false;
}

void NetworkManager::Disconnect()
{
    if (connected && client)
    {
        // 先离开房间
        if (inRoom)
        {
            LeaveRoom();
        }
        // 断开服务器连接
        client->disconnectFromServer();
        connected = false;
    }
}

// ============== 数据发送 ==============
void NetworkManager::SendMyInput(Direction dir)
{
    if (!connected || !client || !inRoom)
    {
        return;
    }

    // 创建GamePacket
    GamePacket packet;
    packet.frame = frameCounter++;
    packet.snakeId = myPlayerId;
    packet.dir = DirectionToChar(dir);
    packet.uid = myUID; // 【关键】带上本客户端的唯一标识

    // 编码为二进制
    char buffer[MAX_BUFFER_LEN];
    int len = 0;
    EncodePacket(packet, buffer, len);

    // 发送
    client->sendBinaryMsg(buffer, len);
}

void NetworkManager::SendGamePacket(const GamePacket &packet)
{
    if (!connected || !client || !inRoom)
    {
        return;
    }

    // 编码并发送
    char buffer[MAX_BUFFER_LEN];
    int len = 0;
    EncodePacket(packet, buffer, len);
    client->sendBinaryMsg(buffer, len);
}

void NetworkManager::SendTextMessage(const std::string &msg)
{
    if (!connected || !client || !inRoom)
    {
        return;
    }

    client->sendMsg(msg.c_str());
}

bool NetworkManager::SendBinaryMessage(const char *data, int len)
{
    if (!connected || !client || !inRoom)
    {
        return false;
    }

    return client->sendBinaryMsg(data, len);
}

// ============== 数据接收 ==============
Direction NetworkManager::GetOpponentInput()
{
    if (receiveQueue.empty())
    {
        return NONE;
    }

    GamePacket packet = receiveQueue.front();
    receiveQueue.pop();
    return CharToDirection(packet.dir);
}

bool NetworkManager::GetOpponentPacket(GamePacket &outPacket)
{
    if (receiveQueue.empty())
    {
        return false;
    }

    outPacket = receiveQueue.front();
    receiveQueue.pop();
    return true;
}

bool NetworkManager::HasPendingMessages() const
{
    return !receiveQueue.empty();
}

// ============== 更新 ==============
void NetworkManager::Update()
{
    // ✅ 所有消息处理都在 ProcessRoomMessages() 中进行
    // 这个方法保留以保持接口兼容性
}

// ============== 私有方法 ==============
void NetworkManager::EncodePacket(const GamePacket &packet, char *buffer, int &outLen)
{
    // 使用二进制编码（直接拷贝结构体）
    memcpy(buffer, &packet, sizeof(GamePacket));
    outLen = sizeof(GamePacket);
}

bool NetworkManager::DecodePacket(const char *data, int len, GamePacket &outPacket)
{
    // 使用二进制解码
    if (len >= (int)sizeof(GamePacket))
    {
        memcpy(&outPacket, data, sizeof(GamePacket));
        return true;
    }
    return false;
}

char NetworkManager::DirectionToChar(Direction dir)
{
    // TODO: 方向转字符
    switch (dir)
    {
    case UP:
        return 'U';
    case DOWN:
        return 'D';
    case LEFT:
        return 'L';
    case RIGHT:
        return 'R';
    default:
        return 'N';
    }
}

Direction NetworkManager::CharToDirection(char c)
{
    // TODO: 字符转方向
    switch (c)
    {
    case 'U':
        return UP;
    case 'D':
        return DOWN;
    case 'L':
        return LEFT;
    case 'R':
        return RIGHT;
    default:
        return NONE;
    }
}

// ============== 全局服务器管理器实现 ==============
bool GlobalServerManager::StartServer()
{
    // 检查 serv.exe 是否已在运行
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(hSnapshot, &pe32))
        {
            do
            {
                if (wcscmp(pe32.szExeFile, L"serv.exe") == 0)
                {
                    // 服务器已在运行
                    CloseHandle(hSnapshot);
                    return true;
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }

    // 尝试从多个可能的位置启动 serv.exe
    const char *possiblePaths[] = {
        "serv.exe",                    // 当前目录
        "..\\..\\Snake\\serv.exe",     // VS 编译输出目录（x64\\Debug）到源码目录
        "..\\..\\..\\Snake\\serv.exe", // 多层嵌套的 Debug 目录
        "Snake\\serv.exe",             // 当前目录下的 Snake 文件夹
        "..\\Snake\\serv.exe"          // 上级目录的 Snake 文件夹
    };

    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW; // 显示控制台窗口（方便调试）

    // 尝试每个可能的路径
    for (const char *path : possiblePaths)
    {
        // 先检查文件是否存在
        if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES)
        {
            char cmdLine[MAX_PATH];
            strcpy_s(cmdLine, path);

            if (CreateProcessA(
                    NULL,               // 应用程序名
                    cmdLine,            // 命令行
                    NULL,               // 进程安全属性
                    NULL,               // 线程安全属性
                    FALSE,              // 继承句柄
                    CREATE_NEW_CONSOLE, // 创建新控制台
                    NULL,               // 环境
                    NULL,               // 当前目录
                    &si,                // 启动信息
                    &pi))               // 进程信息
            {
                SetServerProcess(pi.hProcess);
                CloseHandle(pi.hThread);
                return true;
            }
        }
    }

    // 所有路径都失败
    return false;
}

void GlobalServerManager::StopServer()
{
    if (serverProcess && isServerStartedByUs)
    {
        // 尝试优雅关闭
        TerminateProcess(serverProcess, 0);
        WaitForSingleObject(serverProcess, 2000); // 等待最多2秒
        CloseHandle(serverProcess);
        serverProcess = nullptr;
        isServerStartedByUs = false;
    }
}

// ============== 房间管理实现 ==============
bool NetworkManager::CreateRoom(const std::string &roomName, uint32_t &outRoomId)
{
    if (!connected || !client)
    {
        return false;
    }

    // 调用 Client.h 的 hostNewRoom 方法
    if (client->hostNewRoom(roomName.c_str(), outRoomId))
    {
        currentRoomId = outRoomId;
        inRoom = true;
        isHost = true;
        playerReady = false;
        return true;
    }

    return false;
}

bool NetworkManager::GetRoomList(std::vector<RoomInfo> &outRooms)
{
    if (!connected || !client)
    {
        return false;
    }

    outRooms.clear();

    // 调用 Client.h 的 getRoomList 方法
    NetworkHelper::room_list_t rlt;
    if (client->getRoomList(rlt))
    {
        for (uint32_t i = 0; i < rlt.roomNum; ++i)
        {
            RoomInfo info;
            info.roomId = rlt.rooms[i].roomId;

            // 将 char[] 转换为 wchar_t[]
            MultiByteToWideChar(CP_UTF8, 0, rlt.rooms[i].roomName, -1, info.roomName, 40);

            // 获取房间详细信息以得到房主名称
            NetworkHelper::room_t roomDetail;
            if (client->getRoomInfo(rlt.rooms[i].roomId, roomDetail) && roomDetail.existing > 0)
            {
                // 第一个玩家是房主
                MultiByteToWideChar(CP_UTF8, 0, roomDetail.everyOne[0].name, -1, info.hostName, 40);
            }
            else
            {
                wcscpy_s(info.hostName, 40, L"Unknown");
            }

            info.currentPlayers = rlt.rooms[i].existing;
            info.maxPlayers = 2;
            info.status = (info.currentPlayers >= 2) ? ROOM_FULL : ROOM_WAITING;
            outRooms.push_back(info);
        }
        return true;
    }

    return false;
}

bool NetworkManager::JoinRoom(uint32_t roomId)
{
    if (!connected || !client)
    {
        return false;
    }

    // 调用 Client.h 的 connectToRoom 方法
    if (client->connectToRoom(roomId))
    {
        currentRoomId = roomId;
        inRoom = true;
        isHost = false;
        playerReady = false;
        return true;
    }

    return false;
}

bool NetworkManager::LeaveRoom()
{
    if (!inRoom || !client)
    {
        return false;
    }

    // 调用 Client.h 的 disconnectFromRoom 方法
    if (client->disconnectFromRoom())
    {
        inRoom = false;
        isHost = false;
        playerReady = false;
        currentRoomId = 0;
        return true;
    }

    return false;
}

bool NetworkManager::SetPlayerReady(bool ready)
{
    if (!inRoom || !client)
    {
        return false;
    }

    playerReady = ready;

    // 更新本地玩家的准备状态
    wchar_t playerNameWchar[40] = {0};
    MultiByteToWideChar(CP_UTF8, 0, playerName.c_str(), -1, playerNameWchar, 40);
    std::wstring playerNameWstr(playerNameWchar);
    playerReadyStates[playerNameWstr] = ready;

    // 创建带 UID 的指令包
    GameCommand cmd;
    cmd.cmdType = ready ? CMD_READY : CMD_NOT_READY;
    cmd.data = ready ? 1 : 0;
    cmd.uid = myUID; // 【关键】盖上身份印章

    // 发送二进制消息（包含 UID）
    return client->sendBinaryMsg((char *)&cmd, sizeof(GameCommand));
}

bool NetworkManager::SendLobbyUpdate(const LobbyUpdatePacket &packet)
{
    if (!connected || !inRoom || !client)
    {
        return false;
    }

    // 将 LobbyUpdatePacket 编码为二进制数据并发送
    char buffer[MAX_BUFFER_LEN];
    memcpy(buffer, &packet, sizeof(LobbyUpdatePacket));
    return client->sendBinaryMsg(buffer, sizeof(LobbyUpdatePacket));
}

bool NetworkManager::ReceiveLobbyUpdate(LobbyUpdatePacket &outPacket)
{
    if (!connected || !inRoom || !client)
    {
        return false;
    }

    // 从接收队列中解析大厅更新消息
    NetworkHelper::msg_package_t msgPackage;
    if (client->recvMsg(msgPackage))
    {
        // 查找大厅更新消息（检查消息长度是否匹配）
        for (uint32_t i = 0; i < msgPackage.msgNum; ++i)
        {
            NetworkHelper::msg_t *msg = msgPackage.msgs[i];
            if (msg && msg->msgContent && msg->msgLen >= sizeof(LobbyUpdatePacket))
            {
                memcpy(&outPacket, msg->msgContent, sizeof(LobbyUpdatePacket));
                return true;
            }
        }
    }

    return false;
}

bool NetworkManager::GetCurrentRoomPlayers(std::vector<LobbyPlayerInfo> &outPlayerList)
{
    if (!connected || !client || !inRoom)
    {
        return false;
    }

    outPlayerList.clear();

    // 获取当前房间的详细信息
    NetworkHelper::room_t roomInfo;
    if (client->getRoomInfo(currentRoomId, roomInfo))
    {
        // 转换每个玩家信息
        for (uint32_t i = 0; i < roomInfo.existing; ++i)
        {
            LobbyPlayerInfo playerInfo;

            // 转换昵称
            MultiByteToWideChar(CP_UTF8, 0, roomInfo.everyOne[i].name, -1, playerInfo.nickname, 40);

            // 从准备状态映射中获取实际状态
            std::wstring playerNameWstr(playerInfo.nickname);
            auto it = playerReadyStates.find(playerNameWstr);
            if (it != playerReadyStates.end())
            {
                playerInfo.status = it->second ? PLAYER_READY : PLAYER_NOT_READY;
            }
            else
            {
                // 如果没有记录，默认为未准备
                playerInfo.status = PLAYER_NOT_READY;
            }

            // 设置头像颜色（简单分配）
            playerInfo.avatarColor = (i == 0) ? RGB(63, 114, 175) : RGB(85, 132, 188);
            playerInfo.isHost = (i == 0);

            outPlayerList.push_back(playerInfo);
        }
        return true;
    }

    return false;
}

void NetworkManager::ProcessRoomMessages()
{
    if (!connected || !client || !inRoom)
    {
        return;
    }

    // 处理所有待处理的房间消息
    NetworkHelper::msg_package_t msgPackage;
    while (client->recvMsg(msgPackage))
    {
        for (uint32_t i = 0; i < msgPackage.msgNum; ++i)
        {
            NetworkHelper::msg_t *msg = msgPackage.msgs[i];
            if (!msg || !msg->msgContent || msg->msgLen < sizeof(int))
            {
                continue;
            }

            // 先读取包类型（第一个 int）
            int *packetType = (int *)msg->msgContent;

            // 【处理 HELLO 包】P2 入场通知
            if (*packetType == CMD_HELLO && msg->msgLen >= sizeof(HelloPacket))
            {
                HelloPacket *hello = (HelloPacket *)msg->msgContent;

                // 检查 UID - 忽略自己的消息
                if (hello->uid == myUID)
                {
                    continue;
                }

                // 记录对手 UID
                if (opponentUID == 0)
                {
                    opponentUID = hello->uid;
                }

                // 【房主专用】收到 P2 的 HELLO，标记 P2 已连接
                if (isHost)
                {
                    wchar_t p2Name[40] = {0};
                    MultiByteToWideChar(CP_UTF8, 0, hello->name, -1, p2Name, 40);
                    std::wstring p2NameWstr(p2Name);
                    playerReadyStates[p2NameWstr] = false; // 初始未准备

                    // 更新接收状态
                    receivedLobbyState.hasUpdate = true;
                    receivedLobbyState.p2Connected = true;
                    wcsncpy_s(receivedLobbyState.p2Name, 40, p2Name, 39);
                }
                continue;
            }

            // 【处理 LOBBY_SYNC 包】房主心跳广播
            if (*packetType == CMD_LOBBY_SYNC && msg->msgLen >= sizeof(LobbyStatePacket))
            {
                LobbyStatePacket *state = (LobbyStatePacket *)msg->msgContent;

                // 检查 UID - 忽略自己的消息
                if (state->uid == myUID)
                {
                    continue;
                }

                // 【客机专用】收到房主的状态同步
                if (!isHost)
                {
                    // 更新房主（P1）状态
                    wchar_t p1Name[40] = {0};
                    MultiByteToWideChar(CP_UTF8, 0, state->p1_name, -1, p1Name, 40);
                    std::wstring p1NameWstr(p1Name);
                    playerReadyStates[p1NameWstr] = state->p1_ready;

                    // 更新自己（P2）的状态（从房主确认）
                    wchar_t p2Name[40] = {0};
                    MultiByteToWideChar(CP_UTF8, 0, state->p2_name, -1, p2Name, 40);
                    std::wstring p2NameWstr(p2Name);
                    playerReadyStates[p2NameWstr] = state->p2_ready;

                    // 更新接收状态 - 填充完整的LobbyStatePacket数据
                    receivedLobbyState.hasUpdate = true;
                    receivedLobbyState.receivedLobbySync = true;
                    receivedLobbyState.p1Exist = state->p1_exist;
                    receivedLobbyState.p1Ready = state->p1_ready;
                    receivedLobbyState.p2Exist = state->p2_exist;
                    receivedLobbyState.p2Ready = state->p2_ready;
                    strncpy_s(receivedLobbyState.p1Name, 40, state->p1_name, 39);
                    strncpy_s(receivedLobbyState.p2NameUtf8, 40, state->p2_name, 39);
                }
                continue;
            }

            // 【处理 EXIT 包】玩家退出通知
            if (*packetType == CMD_EXIT && msg->msgLen >= sizeof(ExitPacket))
            {
                ExitPacket *exitPkg = (ExitPacket *)msg->msgContent;

                // 检查 UID - 忽略自己的消息
                if (exitPkg->uid == myUID)
                {
                    continue;
                }

                // ✅ 收到对手的退出通知
                receivedLobbyState.hasUpdate = true;
                receivedLobbyState.exitReceived = true;

                continue;
            }

            // 【处理 GameCommand 包】准备、开始等指令
            if (msg->msgLen >= sizeof(GameCommand))
            {
                GameCommand *cmd = (GameCommand *)msg->msgContent;

                // 检查 UID - 忽略自己的消息
                if (cmd->uid == myUID)
                {
                    continue;
                }

                // 记录对手 UID
                if (opponentUID == 0)
                {
                    opponentUID = cmd->uid;
                }

                // 获取发送者名字
                wchar_t senderName[40] = {0};
                MultiByteToWideChar(CP_UTF8, 0, msg->name, -1, senderName, 40);
                std::wstring senderNameWstr(senderName);

                // 处理不同类型的指令
                switch (cmd->cmdType)
                {
                case CMD_READY:
                    // 对手准备了
                    playerReadyStates[senderNameWstr] = true;

                    // 【关键修复】P1 收到 P2 的准备消息时，立即更新状态
                    if (isHost)
                    {
                        receivedLobbyState.hasUpdate = true;
                        receivedLobbyState.p2Ready = true;
                    }
                    break;

                case CMD_NOT_READY:
                    // 对手取消准备
                    playerReadyStates[senderNameWstr] = false;

                    // 【关键修复】P1 收到 P2 取消准备消息时，立即更新状态
                    if (isHost)
                    {
                        receivedLobbyState.hasUpdate = true;
                        receivedLobbyState.p2Ready = false;
                    }
                    break;

                case CMD_START_GAME:
                    // P2 接收到房主发起的游戏开始命令
                    if (!IsMyMessage(cmd->uid))
                    {
                        receivedLobbyState.gameStarted = true;
                        receivedLobbyState.hasUpdate = true;
                    }
                    break;

                case CMD_INPUT:
                    // 对手的输入（游戏中）
                    if (!IsMyMessage(cmd->uid))
                    {
                        receivedLobbyState.remoteInput = cmd->data; // 保存对手的方向（0-3）
                        receivedLobbyState.hasUpdate = true;
                    }
                    break;

                case CMD_GAME_OVER:
                    // 对手发送游戏结束消息
                    if (!IsMyMessage(cmd->uid))
                    {
                        receivedLobbyState.gameOver = true;
                        receivedLobbyState.opponentWon = (cmd->data == 1); // 对手是否胜利
                        receivedLobbyState.hasUpdate = true;
                    }
                    break;

                default:
                    break;
                }
            }
        }
    }
}

bool NetworkManager::GetReceivedLobbyState(ReceivedLobbyState &outState)
{
    if (receivedLobbyState.hasUpdate)
    {
        outState = receivedLobbyState;

        // 标记已读
        receivedLobbyState.hasUpdate = false;

        // 【核心修复】消费掉"一次性事件"
        // 这样下一次loop如果没收到新包，或者收到的是无关包，
        // 就不会再次触发开始游戏或退出了
        receivedLobbyState.gameStarted = false;
        receivedLobbyState.exitReceived = false;
        receivedLobbyState.gameOver = false;
        receivedLobbyState.receivedLobbySync = false; // LOBBY_SYNC也是一次性的

        // 注意：p1Ready/p2Ready 是持久状态，不要在这里清空，
        // 它们只能通过 ResetRoomState 或 收到 CMD_NOT_READY 来改变

        return true;
    }
    return false;
}

void NetworkManager::ResetRoomState()
{
    // 清空接收状态结构体
    receivedLobbyState.hasUpdate = false;
    receivedLobbyState.gameStarted = false; // 核心：清除开始标志
    receivedLobbyState.gameOver = false;
    receivedLobbyState.exitReceived = false;
    receivedLobbyState.opponentWon = false;
    receivedLobbyState.remoteInput = -1;
    receivedLobbyState.receivedLobbySync = false;

    // 准备状态也要重置
    receivedLobbyState.p1Ready = false;
    receivedLobbyState.p2Ready = false;
    receivedLobbyState.p1Exist = false;
    receivedLobbyState.p2Exist = false;
    receivedLobbyState.p2Connected = false;

    // 清空内部记录的玩家准备表
    playerReadyStates.clear();

    // 重置本地逻辑状态
    playerReady = false;

    // 注意：不要清空 myUID 或 opponentUID，也不要断开连接
}

void NetworkManager::SendStartGameCommand()
{
    GameCommand cmd;
    cmd.cmdType = CMD_START_GAME;
    cmd.data = 0;
    cmd.uid = myUID;

    SendBinaryMessage((char *)&cmd, sizeof(GameCommand));
}

Direction NetworkManager::GetRemoteDirection()
{
    // 从 receivedLobbyState 获取远程输入
    if (receivedLobbyState.remoteInput >= 0 && receivedLobbyState.remoteInput <= 3)
    {
        Direction dir = (Direction)receivedLobbyState.remoteInput;
        // ✅ 清除已读取的输入，避免重复使用
        // NetworkController会缓存这个方向，直到收到新的输入
        receivedLobbyState.remoteInput = -1;
        return dir;
    }
    return NONE;
}
