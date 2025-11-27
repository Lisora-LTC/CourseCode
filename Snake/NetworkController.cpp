#include "NetworkController.h"
#include "NetworkManager.h"
#include "Snake.h"
#include "GameMap.h"

// ============== 构造与析构 ==============
NetworkController::NetworkController()
    : networkMgr(nullptr), cachedDirection(NONE), opponentId(-1)
{
    // TODO: 初始化
}

NetworkController::NetworkController(NetworkManager *netMgr, int oppId)
    : networkMgr(netMgr), cachedDirection(NONE), opponentId(oppId)
{
    // TODO: 初始化
}

NetworkController::~NetworkController()
{
    // 注意：不delete networkMgr，生命周期由外部管理
}

// ============== 实现接口方法 ==============
Direction NetworkController::MakeDecision(const Snake &snake, const GameMap &map)
{
    // TODO: 返回缓存的方向
    // 这个方向是通过 Update() 从网络获取的
    return cachedDirection;
}

void NetworkController::Update()
{
    // TODO: 每帧调用，从网络管理器获取最新方向
    // cachedDirection = FetchDirectionFromNetwork();
}

void NetworkController::Init()
{
    // TODO: 初始化控制器
    cachedDirection = NONE;
}

// ============== 私有方法 ==============
Direction NetworkController::FetchDirectionFromNetwork()
{
    // TODO: 从网络管理器的接收队列中获取对手的方向
    // 如果没有新数据，返回上一次的方向

    return NONE;
}
