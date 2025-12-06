#include "AIController.h"
#include "Snake.h"
#include "GameMap.h"
#include "FoodManager.h"
#include <queue>
#include <map>
#include <algorithm>
#include <cmath>

// ============== 构造与析构 ==============
AIController::AIController() : nextMove(NONE), hasPath(false), targetFood(-1, -1), foodManager(nullptr)
{
}

AIController::AIController(FoodManager *foodMgr) : nextMove(NONE), hasPath(false), targetFood(-1, -1), foodManager(foodMgr)
{
}

AIController::~AIController()
{
    // 清理路径队列
    while (!path.empty())
        path.pop();
    // 注意：不delete foodManager，生命周期由外部管理
    foodManager = nullptr;
}

// ============== 实现接口方法 ==============
Direction AIController::MakeDecision(const Snake &snake, const GameMap &map)
{
    Point head = snake.GetHead();

    // 1. 如果有预计算的路径且路径有效，返回路径的下一步
    if (!path.empty())
    {
        Direction nextDir = path.front();
        path.pop();

        // 验证这个方向是否安全
        Point nextPos = Utils::GetNextPoint(head, nextDir);
        if (IsSafePosition(nextPos, snake, map))
        {
            nextMove = nextDir;
            return nextDir;
        }
        else
        {
            // 路径失效，清空队列
            while (!path.empty())
                path.pop();
            hasPath = false;
        }
    }

    // 2. 尝试寻找最近的食物并规划路径
    if (foodManager)
    {
        Point nearestFood = FindNearestFood(head);

        // 如果找到食物且不是当前目标，尝试规划新路径
        if (nearestFood.x >= 0 && nearestFood.y >= 0)
        {
            // 如果目标食物改变或没有路径，重新规划
            if (!hasPath || targetFood != nearestFood)
            {
                if (FindPathToFood(snake, map, nearestFood))
                {
                    // 成功找到路径，返回第一步
                    if (!path.empty())
                    {
                        Direction nextDir = path.front();
                        path.pop();
                        nextMove = nextDir;
                        return nextDir;
                    }
                }
            }
        }
    }

    // 3. 如果无法到达食物或没有食物，选择最安全的方向
    Direction safeDir = ChooseSafeDirection(snake, map);
    nextMove = safeDir;
    return safeDir;
}

void AIController::Update()
{
    // 每帧更新（可用于重新规划路径）
}

void AIController::Init()
{
    nextMove = NONE;
    hasPath = false;
    targetFood = Point(-1, -1);
    while (!path.empty())
        path.pop();
}

// ============== 私有方法 ==============
bool AIController::FindPathToFood(const Snake &snake, const GameMap &map, const Point &foodPos)
{
    // 使用BFS算法寻找路径
    Point head = snake.GetHead();

    // 如果食物位置无效
    if (!Utils::IsInBounds(foodPos))
        return false;

    // BFS队列
    std::queue<Point> queue;
    std::map<Point, Point> parent; // 记录路径：parent[child] = parent_point
    std::map<Point, bool> visited;

    queue.push(head);
    visited[head] = true;
    parent[head] = Point(-1, -1); // 起点没有父节点

    // BFS搜索
    Direction directions[] = {UP, DOWN, LEFT, RIGHT};
    bool found = false;

    while (!queue.empty() && !found)
    {
        Point current = queue.front();
        queue.pop();

        // 找到食物
        if (current == foodPos)
        {
            found = true;
            break;
        }

        // 尝试四个方向
        for (Direction dir : directions)
        {
            Point next = Utils::GetNextPoint(current, dir);

            // 检查是否已访问或不安全
            if (visited.find(next) != visited.end() && visited[next])
                continue;

            if (!IsSafePosition(next, snake, map))
                continue;

            queue.push(next);
            visited[next] = true;
            parent[next] = current;
        }
    }

    // 如果没找到路径
    if (!found)
        return false;

    // 回溯路径并转换为方向序列
    std::vector<Direction> pathDirs;
    Point current = foodPos;

    while (parent[current] != Point(-1, -1))
    {
        Point prev = parent[current];

        // 根据两点确定方向
        if (current.x > prev.x)
            pathDirs.push_back(RIGHT);
        else if (current.x < prev.x)
            pathDirs.push_back(LEFT);
        else if (current.y > prev.y)
            pathDirs.push_back(DOWN);
        else if (current.y < prev.y)
            pathDirs.push_back(UP);

        current = prev;
    }

    // 反转路径（因为是从食物回溯到头部）
    std::reverse(pathDirs.begin(), pathDirs.end());

    // 清空旧路径，添加新路径
    while (!path.empty())
        path.pop();

    for (Direction dir : pathDirs)
    {
        path.push(dir);
    }

    hasPath = true;
    targetFood = foodPos;
    return true;
}

bool AIController::FindPathAStar(const Snake &snake, const GameMap &map, const Point &foodPos)
{
    // A*算法（可选实现，暂时使用BFS）
    return FindPathToFood(snake, map, foodPos);
}

int AIController::EvaluateDirection(const Point &head, Direction dir, const Snake &snake, const GameMap &map)
{
    Point next = Utils::GetNextPoint(head, dir);

    // 1. 如果不安全，返回极低分数
    if (!IsSafePosition(next, snake, map))
        return -1000;

    int score = 0;

    // 2. 计算前方可达空间（使用BFS）
    std::queue<Point> queue;
    std::map<Point, bool> visited;
    int reachableSpace = 0;
    int maxDepth = 10; // 限制搜索深度，避免性能问题

    queue.push(next);
    visited[next] = true;

    Direction directions[] = {UP, DOWN, LEFT, RIGHT};

    while (!queue.empty() && reachableSpace < maxDepth * 4)
    {
        Point current = queue.front();
        queue.pop();
        reachableSpace++;

        for (Direction d : directions)
        {
            Point neighbor = Utils::GetNextPoint(current, d);

            if (visited.find(neighbor) != visited.end() && visited[neighbor])
                continue;

            if (!IsSafePosition(neighbor, snake, map))
                continue;

            queue.push(neighbor);
            visited[neighbor] = true;
        }
    }

    score += reachableSpace * 10; // 可达空间越大，分数越高

    // 3. 避免频繁转向（保持当前方向有加分）
    if (dir == snake.GetDirection())
        score += 5;

    return score;
}

Direction AIController::ChooseSafeDirection(const Snake &snake, const GameMap &map)
{
    Point head = snake.GetHead();
    Direction currentDir = snake.GetDirection();

    Direction directions[] = {UP, DOWN, LEFT, RIGHT};
    Direction bestDir = currentDir;
    int bestScore = -10000;

    for (Direction dir : directions)
    {
        // 不能180度转向
        if (Utils::IsOppositeDirection(dir, currentDir))
            continue;

        int score = EvaluateDirection(head, dir, snake, map);

        if (score > bestScore)
        {
            bestScore = score;
            bestDir = dir;
        }
    }

    // 如果所有方向都不安全，保持当前方向
    if (bestScore == -10000)
        return currentDir;

    return bestDir;
}

Point AIController::FindNearestFood(const Point &head)
{
    if (!foodManager)
        return Point(-1, -1);

    const std::vector<Food> &foods = foodManager->GetAllFoods();

    if (foods.empty())
        return Point(-1, -1);

    Point nearest = foods[0].position;
    int minDist = abs(head.x - nearest.x) + abs(head.y - nearest.y); // 曼哈顿距离

    for (size_t i = 1; i < foods.size(); i++)
    {
        int dist = abs(head.x - foods[i].position.x) + abs(head.y - foods[i].position.y);
        if (dist < minDist)
        {
            minDist = dist;
            nearest = foods[i].position;
        }
    }

    return nearest;
}

bool AIController::IsSafePosition(const Point &pos, const Snake &snake, const GameMap &map)
{
    // 1. 检查是否在边界内
    if (!Utils::IsInBounds(pos))
        return false;

    // 2. 检查是否撞墙
    if (map.IsWall(pos))
        return false;

    // 3. 检查是否撞蛇身（除了尾巴，因为尾巴会移动）
    const std::vector<Point> &body = snake.GetBody();
    for (size_t i = 0; i < body.size() - 1; i++) // 不检查最后一个（尾巴）
    {
        if (pos == body[i])
            return false;
    }

    return true;
}
