#pragma once
// ============== Client.h ==============
// 这是助教提供的网络框架代码
// 请将助教提供的完整 Client.h 代码放在这里
// 不要修改这个文件（除非修Bug）

// TODO: 复制粘贴助教提供的网络框架代码到这里

/*
 * 预期的类结构（参考）：
 *
 * class ClientHelper {
 * public:
 *     bool Connect(const char* serverIp, int port, const char* username);
 *     bool SendMsg(const char* msg);
 *     bool SendBinaryMsg(const char* data, int len);
 *     msg_t ReceiveMsg();
 *     bool HasMessage();
 *     void Disconnect();
 * };
 *
 * struct msg_t {
 *     int from;
 *     int length;
 *     char* body;
 * };
 */

// 临时占位结构（请替换为助教提供的真实代码）
struct msg_t
{
    int from;
    int length;
    char *body;
};

class ClientHelper
{
public:
    bool Connect(const char *serverIp, int port, const char *username) { return false; }
    bool SendMsg(const char *msg) { return false; }
    bool SendBinaryMsg(const char *data, int len) { return false; }
    msg_t ReceiveMsg() { return msg_t(); }
    bool HasMessage() { return false; }
    void Disconnect() {}
};
