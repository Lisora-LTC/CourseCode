#include "AIController.h"
#include "Snake.h"
#include "GameMap.h"
#include "Utils.h"

// ============== 构造与析构 ==============
AIController::AIController() : nextMove(NONE), hasPath(false)
{
    // TODO: 初始化
}

AIController::~AIController()
{
    // TODO: 清理
}

// ============== 实现接口方法 ==============
Direction AIController::MakeDecision(const Snake &snake, const GameMap &map)
{
    // TODO: AI决策逻辑
    // 1. 如果有预计算的路径，返回路径的下一步
    // 2. 如果没有路径，重新计算路径到最近的食物
    // 3. 如果无法到达食物，选择最安全的方向

    return NONE; // 临时返回
}

void AIController::Update()
{
    // TODO: 每帧更新（可选）
}

void AIController::Init()
{
    // TODO: 初始化
    nextMove = NONE;
    hasPath = false;
    while (!path.empty())
        path.pop();
}

// ============== 私有方法 ==============
bool AIController::FindPathToFood(const Snake &snake, const GameMap &map, const Point &foodPos)
{
    // TODO: 使用BFS算法寻找路径
    // 1. 从蛇头开始BFS搜索
    // 2. 记录路径
    // 3. 将路径转换为方向序列存入path队列

    return false;
}

bool AIController::FindPathAStar(const Snake &snake, const GameMap &map, const Point &foodPos)
{
    // TODO: 使用A*算法寻找路径（可选实现，性能更优）

    return false;
}

int AIController::EvaluateDirection(const Point &head, Direction dir, const Snake &snake, const GameMap &map)
{
    // TODO: 评估某个方向的安全性
    // 返回一个分数，分数越高越安全
    // 考虑因素：
    // 1. 是否撞墙
    // 2. 是否撞蛇
    // 3. 前方的可达空间大小（避免走进死路）

    return 0;
}

Direction AIController::ChooseSafeDirection(const Snake &snake, const GameMap &map)
{
    // TODO: 选择最安全的方向
    // 遍历四个方向，选择评分最高的

    return NONE;
}

bool AIController::IsSafePosition(const Point &pos, const Snake &snake, const GameMap &map)
{
    // TODO: 判断位置是否安全
    // 1. 检查是否在边界内
    // 2. 检查是否撞墙
    // 3. 检查是否撞蛇身

    return false;
}
