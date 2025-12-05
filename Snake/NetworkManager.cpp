#include "NetworkManager.h"
#include "Client.h"
#include <cstring>

// ============== 构造与析构 ==============
NetworkManager::NetworkManager()
    : client(nullptr), connected(false), myPlayerId(-1), frameCounter(0), serverPort(0)
{
    // TODO: 初始化
}

NetworkManager::~NetworkManager()
{
    Disconnect();
    if (client)
    {
        delete client;
        client = nullptr;
    }
}

// ============== 连接管理 ==============
bool NetworkManager::Connect(const std::string &ip, int port, const std::string &name)
{
    // TODO: 连接到服务器
    // return client.Connect(ip.c_str(), port, name.c_str());

    serverIp = ip;
    serverPort = port;
    playerName = name;
    connected = false; // 临时
    return false;
}

void NetworkManager::Disconnect()
{
    // TODO: 断开连接
    if (connected)
    {
        // client.Disconnect();
        connected = false;
    }
}

// ============== 数据发送 ==============
void NetworkManager::SendMyInput(Direction dir)
{
    // TODO: 发送我的输入
    // 1. 创建GamePacket
    // 2. 编码为二进制
    // 3. 调用 client.SendBinaryMsg()
}

void NetworkManager::SendGamePacket(const GamePacket &packet)
{
    // TODO: 发送游戏数据包
    // 编码并发送
}

void NetworkManager::SendTextMessage(const std::string &msg)
{
    // TODO: 发送文本消息
    // client.SendMsg(msg.c_str());
}

// ============== 数据接收 ==============
Direction NetworkManager::GetOpponentInput()
{
    // TODO: 从接收队列获取对手的方向
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
    // TODO: 从接收队列获取对手的数据包
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
    // TODO: 检查是否有待处理消息
    // return client.HasMessage();
    return false;
}

// ============== 更新 ==============
void NetworkManager::Update()
{
    // TODO: 每帧调用，接收网络数据
    // 1. 检查 client.HasMessage()
    // 2. 接收消息 client.ReceiveMsg()
    // 3. 解码并放入 receiveQueue

    // while (client.HasMessage()) {
    //     msg_t msg = client.ReceiveMsg();
    //     GamePacket packet;
    //     if (DecodePacket(msg.body, msg.length, packet)) {
    //         receiveQueue.push(packet);
    //     }
    // }
}

// ============== 私有方法 ==============
void NetworkManager::EncodePacket(const GamePacket &packet, char *buffer, int &outLen)
{
    // TODO: 编码数据包
    // 方案1：直接memcpy结构体
    // memcpy(buffer, &packet, sizeof(GamePacket));
    // outLen = sizeof(GamePacket);

    // 方案2：按字段编码（更安全，跨平台）
    // sprintf(buffer, "%d %d %d %d %c", packet.frame, packet.snakeId, packet.x, packet.y, packet.dir);
}

bool NetworkManager::DecodePacket(const char *data, int len, GamePacket &outPacket)
{
    // TODO: 解码数据包
    // 方案1：直接memcpy
    // if (len >= sizeof(GamePacket)) {
    //     memcpy(&outPacket, data, sizeof(GamePacket));
    //     return true;
    // }

    // 方案2：按字段解码
    // sscanf(data, "%d %d %d %d %c", &outPacket.frame, &outPacket.snakeId,
    //        &outPacket.x, &outPacket.y, &outPacket.dir);

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
