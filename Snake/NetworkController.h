#pragma once
#include "IController.h"
#include <queue>

// 前向声明
class NetworkManager;

// ============== 网络控制器 ==============
// 用于接收网络对手的操作
class NetworkController : public IController
{
private:
    NetworkManager *networkMgr; // 网络管理器引用
    Direction cachedDirection;  // 缓存的方向
    int opponentId;             // 对手ID

public:
    // ============== 构造与析构 ==============
    NetworkController();
    NetworkController(NetworkManager *netMgr, int oppId);
    ~NetworkController() override;

    // ============== 实现接口方法 ==============
    Direction MakeDecision(const Snake &snake, const GameMap &map) override;
    void Update() override;
    void Init() override;
    const char *GetTypeName() const override { return "NetworkController"; }

private:
    /**
     * @brief 从网络管理器获取最新方向
     */
    Direction FetchDirectionFromNetwork();
};
