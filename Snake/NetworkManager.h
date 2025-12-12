#pragma once
#include "Common.h"
#include <queue>
#include <string>
#include <map>
#include <windows.h>

// 前向声明
namespace NetworkHelper
{
    class ClientHelper;
}

// ============== 全局服务器管理器 ==============
// 使用单例模式管理 serv.exe 进程，确保程序退出时自动清理
class GlobalServerManager
{
private:
    HANDLE serverProcess;
    bool isServerStartedByUs;

    GlobalServerManager() : serverProcess(nullptr), isServerStartedByUs(false) {}

public:
    ~GlobalServerManager()
    {
        // 无论程序如何退出（正常退出、点X关闭、崩溃），这里都会执行
        StopServer();
    }

    static GlobalServerManager &GetInstance()
    {
        static GlobalServerManager instance;
        return instance;
    }

    // 启动服务器
    bool StartServer();

    // 停止服务器
    void StopServer();

    // 设置服务器进程句柄
    void SetServerProcess(HANDLE handle)
    {
        serverProcess = handle;
        isServerStartedByUs = true;
    }

    // 禁用拷贝
    GlobalServerManager(const GlobalServerManager &) = delete;
    GlobalServerManager &operator=(const GlobalServerManager &) = delete;
};

// ============== 游戏数据包结构 ==============
// 定义客户端与服务器之间的通信协议
struct GamePacket
{
    int frame;        // 帧号（用于同步）
    int snakeId;      // 蛇的ID
    int x;            // X坐标
    int y;            // Y坐标
    char dir;         // 方向 ('U'=上, 'D'=下, 'L'=左, 'R'=右)
    unsigned int uid; // 发送者的唯一标识符（Session ID）

    GamePacket() : frame(0), snakeId(0), x(0), y(0), dir('N'), uid(0) {}
};

// ============== 游戏指令结构 ==============
// 用于房间内的指令通信（准备、开始等）
struct GameCommand
{
    int cmdType;      // 指令类型（见下方常量定义）
    int data;         // 附加数据（用途取决于cmdType）
    unsigned int uid; // 发送者的唯一标识符

    GameCommand() : cmdType(0), data(0), uid(0) {}
};

// 指令类型常量
const int CMD_READY = 0;        // 准备
const int CMD_NOT_READY = 1;    // 取消准备
const int CMD_START_GAME = 2;   // 开始游戏
const int CMD_INPUT = 3;        // 输入方向（data = Direction枚举值）
const int CMD_GAME_OVER = 7;    // 游戏结束
const int CMD_LOBBY_SYNC = 101; // 大厅状态同步（房主心跳广播）
const int CMD_HELLO = 102;      // P2 入场通知
const int CMD_EXIT = 104;       // 离开房间/房间解散

// ✅ 说明：目前使用 GameCommand 统一处理所有游戏消息
// - 输入同步：通过 CMD_INPUT 发送方向（每次方向改变时）
// - data 字段存储 Direction 枚举值（UP=0, DOWN=1, LEFT=2, RIGHT=3）

// ============== 大厅状态包 ==============
// 用于房主定期广播房间状态
struct LobbyStatePacket
{
    int packetType;   // CMD_LOBBY_SYNC (101)
    unsigned int uid; // 发送者 UID

    // 玩家1（房主）信息
    bool p1_exist;
    bool p1_ready;
    char p1_name[40];

    // 玩家2（客机）信息
    bool p2_exist;
    bool p2_ready;
    char p2_name[40];

    LobbyStatePacket()
        : packetType(CMD_LOBBY_SYNC), uid(0),
          p1_exist(false), p1_ready(false),
          p2_exist(false), p2_ready(false)
    {
        memset(p1_name, 0, sizeof(p1_name));
        memset(p2_name, 0, sizeof(p2_name));
    }
};

// ============== 入场通知包 ==============
// P2 连接时发送，告诉 P1 "我来了"
struct HelloPacket
{
    int packetType;   // CMD_HELLO (102)
    unsigned int uid; // 发送者 UID
    char name[40];    // 玩家名

    HelloPacket() : packetType(CMD_HELLO), uid(0)
    {
        memset(name, 0, sizeof(name));
    }
};

// ============== 退出通知包 ==============
// 玩家离开房间时发送
struct ExitPacket
{
    int packetType;   // CMD_EXIT (104)
    unsigned int uid; // 发送者 UID

    ExitPacket() : packetType(CMD_EXIT), uid(0) {}
};

// ============== 网络管理器 ==============
// 游戏逻辑与底层网络的适配器
class NetworkManager
{
private:
    NetworkHelper::ClientHelper *client; // 助教提供的网络客户端
    std::queue<GamePacket> receiveQueue; // 接收队列
    std::queue<GamePacket> sendQueue;    // 发送队列

    bool connected;
    int myPlayerId;
    int frameCounter;

    std::string serverIp;
    int serverPort;
    std::string playerName;

    // UID 相关：用于区分本地和远程玩家
    unsigned int myUID;       // 本客户端的唯一标识符
    unsigned int opponentUID; // 对手的唯一标识符（收到第一条对方消息时记录）

public:
    // ============== 构造与析构 ==============
    NetworkManager();
    ~NetworkManager();

    // ============== UID 管理 ==============
    /**
     * @brief 生成本客户端的唯一 UID
     * @note 必须在程序启动时调用一次
     */
    void InitUID();

    /**
     * @brief 获取本客户端的 UID
     */
    unsigned int GetMyUID() const { return myUID; }

    /**
     * @brief 获取对手的 UID
     */
    unsigned int GetOpponentUID() const { return opponentUID; }

    /**
     * @brief 检查消息是否来自本客户端
     * @param uid 消息中的 UID
     * @return true 表示是自己发的消息，false 表示是对手发的
     */
    bool IsMyMessage(unsigned int uid) const { return uid == myUID; }

    // ============== 连接管理 ==============
    /**
     * @brief 连接到服务器
     * @param ip 服务器IP地址
     * @param port 端口号
     * @param name 玩家名称
     * @return 是否连接成功
     */
    bool Connect(const std::string &ip, int port, const std::string &name);

    /**
     * @brief 断开连接
     */
    void Disconnect();

    /**
     * @brief 是否已连接
     */
    bool IsConnected() const { return connected; }

    // ============== 数据发送 ==============
    /**
     * @brief 发送我的输入（方向）
     */
    void SendMyInput(Direction dir);

    /**
     * @brief 发送游戏状态包
     */
    void SendGamePacket(const GamePacket &packet);

    /**
     * @brief 发送文本消息
     */
    void SendTextMessage(const std::string &msg);

    /**
     * @brief 发送二进制消息（用于发送结构体）
     */
    bool SendBinaryMessage(const char *data, int len);

    // ============== 数据接收 ==============
    /**
     * @brief 获取对手的输入
     * @return 对手的方向
     */
    Direction GetOpponentInput();

    /**
     * @brief 获取对手的游戏包
     */
    bool GetOpponentPacket(GamePacket &outPacket);

    /**
     * @brief 检查是否有待处理的消息
     */
    bool HasPendingMessages() const;

    // ============== 更新 ==============
    /**
     * @brief 每帧调用，驱动底层接收
     */
    void Update();

    // ============== 房间管理 ==============
    /**
     * @brief 创建房间
     * @param roomName 房间名称
     * @param outRoomId 返回创建的房间ID
     * @return 是否创建成功
     */
    bool CreateRoom(const std::string &roomName, uint32_t &outRoomId);

    /**
     * @brief 获取房间列表
     * @param outRooms 返回房间列表
     * @return 是否获取成功
     */
    bool GetRoomList(std::vector<RoomInfo> &outRooms);

    /**
     * @brief 加入房间
     * @param roomId 要加入的房间ID
     * @return 是否加入成功
     */
    bool JoinRoom(uint32_t roomId);

    /**
     * @brief 离开房间
     * @return 是否离开成功
     */
    bool LeaveRoom();

    /**
     * @brief 设置玩家准备状态
     * @param ready 是否准备
     * @return 是否设置成功
     */
    bool SetPlayerReady(bool ready);

    /**
     * @brief 发送大厅状态更新
     * @param packet 大厅更新数据包
     * @return 是否发送成功
     */
    bool SendLobbyUpdate(const LobbyUpdatePacket &packet);

    /**
     * @brief 接收大厅状态更新
     * @param outPacket 返回大厅更新数据包
     * @return 是否有新的更新
     */
    bool ReceiveLobbyUpdate(LobbyUpdatePacket &outPacket);

    /**
     * @brief 获取当前房间ID
     */
    uint32_t GetCurrentRoomId() const { return currentRoomId; }

    /**
     * @brief 是否在房间中
     */
    bool IsInRoom() const { return inRoom; }

    /**
     * @brief 是否为房主
     */
    bool IsHost() const { return isHost; }

    /**
     * @brief 获取当前房间的详细信息（包括所有玩家）
     * @param outPlayerList 返回玩家列表
     * @return 是否获取成功
     */
    bool GetCurrentRoomPlayers(std::vector<LobbyPlayerInfo> &outPlayerList);

    // ============== 大厅状态结构体（公共） ==============
    // 接收到的大厅状态（用于 LobbyScene 查询）
    struct ReceivedLobbyState
    {
        bool hasUpdate;
        bool p2Connected;   // P2 是否连接（用于房主）
        wchar_t p2Name[40]; // P2 名字（用于房主）
        char p1Name[40];    // P1 名字 UTF-8（用于客机）
        bool p1Ready;       // P1 准备状态（用于客机）
        bool p2Ready;       // P2 准备状态（用于客机）
        bool exitReceived;  // 是否收到退出通知
        bool gameStarted;   // 游戏开始标志（P2 接收到开始命令）
        int remoteInput;    // 远程输入方向（0-3 对应 UP/DOWN/LEFT/RIGHT）
        bool gameOver;      // 游戏结束标志
        bool opponentWon;   // 对手是否胜利

        // LobbyStatePacket 完整数据
        bool receivedLobbySync; // 是否收到LOBBY_SYNC包
        bool p1Exist;           // P1是否存在
        bool p2Exist;           // P2是否存在
        char p2NameUtf8[40];    // P2名字UTF-8（用于客机）

        ReceivedLobbyState() : hasUpdate(false), p2Connected(false), p1Ready(false), p2Ready(false), exitReceived(false), gameStarted(false), remoteInput(-1), gameOver(false), opponentWon(false), receivedLobbySync(false), p1Exist(false), p2Exist(false)
        {
            wmemset(p2Name, 0, 40);
            memset(p1Name, 0, 40);
            memset(p2NameUtf8, 0, 40);
        }
    };

public:
    /**
     * @brief 处理接收到的房间消息，更新准备状态
     */
    void ProcessRoomMessages();

    /**
     * @brief 获取接收到的大厅状态更新
     * @param outState 输出状态
     * @return 是否有新的更新
     */
    bool GetReceivedLobbyState(ReceivedLobbyState &outState);

    /**
     * @brief 发送开始游戏命令（房主调用）
     */
    void SendStartGameCommand();

    /**
     * @brief 获取远程输入方向（NetworkController 调用）
     */
    Direction GetRemoteDirection();

    /**
     * @brief 重置房间状态（用于一局结束后清理残留数据）
     */
    void ResetRoomState();

private:
    // 房间相关状态
    uint32_t currentRoomId; // 当前所在房间ID
    bool inRoom;            // 是否在房间中
    bool isHost;            // 是否为房主
    bool playerReady;       // 玩家是否准备

    // 玩家准备状态映射（玩家名 -> 准备状态）
    std::map<std::wstring, bool> playerReadyStates;

    // 接收到的大厅状态实例
    ReceivedLobbyState receivedLobbyState;

private:
    /**
     * @brief 将GamePacket编码为二进制数据
     */
    void EncodePacket(const GamePacket &packet, char *buffer, int &outLen);

    /**
     * @brief 将二进制数据解码为GamePacket
     */
    bool DecodePacket(const char *data, int len, GamePacket &outPacket);

    /**
     * @brief 将Direction转换为字符
     */
    char DirectionToChar(Direction dir);

    /**
     * @brief 将字符转换为Direction
     */
    Direction CharToDirection(char c);
};
