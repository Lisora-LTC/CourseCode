#include "NetworkController.h"
#include "NetworkManager.h"
#include "Snake.h"
#include "GameMap.h"

// ============== 构造与析构 ==============
NetworkController::NetworkController()
    : networkMgr(nullptr), cachedDirection(NONE), opponentId(-1)
{
}

NetworkController::NetworkController(NetworkManager *netMgr, int oppId)
    : networkMgr(netMgr), cachedDirection(NONE), opponentId(oppId)
{
}

NetworkController::~NetworkController()
{
    // 注意：不delete networkMgr，生命周期由外部管理
    networkMgr = nullptr;
}

// ============== 实现接口方法 ==============
Direction NetworkController::MakeDecision(const Snake &snake, const GameMap &map)
{
    // 返回缓存的方向（这个方向是通过 Update() 从网络获取的）

    // 如果没有有效方向，保持蛇的当前方向
    if (cachedDirection == NONE)
    {
        return snake.GetDirection();
    }

    return cachedDirection;
}

void NetworkController::Update()
{
    // 每帧调用，从网络管理器获取最新方向
    if (networkMgr != nullptr)
    {
        Direction newDir = FetchDirectionFromNetwork();

        // 只在获取到有效方向时更新
        if (newDir != NONE)
        {
            cachedDirection = newDir;
        }
    }
}

void NetworkController::Init()
{
    cachedDirection = NONE;
}

// ============== 私有方法 ==============
Direction NetworkController::FetchDirectionFromNetwork()
{
    // 从网络管理器的接收队列中获取对手的方向
    if (networkMgr == nullptr)
        return NONE;

    // 调用网络管理器的接口获取方向
    // 注意：这里需要NetworkManager实现相应的接口
    // 例如：Direction dir = networkMgr->GetOpponentDirection(opponentId);

    // 暂时返回NONE，等待NetworkManager实现
    // TODO: 实现网络管理器的GetOpponentDirection接口后，替换这里

    return NONE;
}
