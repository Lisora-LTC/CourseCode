#pragma once
#include "Common.h"
#include <queue>
#include <string>

// 前向声明
namespace NetworkHelper
{
    class ClientHelper;
}

// ============== 游戏数据包结构 ==============
// 定义客户端与服务器之间的通信协议
struct GamePacket
{
    int frame;   // 帧号（用于同步）
    int snakeId; // 蛇的ID
    int x;       // X坐标
    int y;       // Y坐标
    char dir;    // 方向 ('U'=上, 'D'=下, 'L'=左, 'R'=右)

    GamePacket() : frame(0), snakeId(0), x(0), y(0), dir('N') {}
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

public:
    // ============== 构造与析构 ==============
    NetworkManager();
    ~NetworkManager();

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
