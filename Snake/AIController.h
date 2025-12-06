#pragma once
#include "IController.h"
#include <queue>
#include <vector>

// 前向声明
class FoodManager;

// ============== AI控制器（加分项） ==============
// 使用BFS或A*算法寻找通往食物的路径
class AIController : public IController
{
private:
    Direction nextMove;         // 下一步移动方向
    std::queue<Direction> path; // 预计算的路径
    Point targetFood;           // 目标食物位置
    bool hasPath;               // 是否有有效路径
    FoodManager *foodManager;   // 食物管理器引用

public:
    // ============== 构造与析构 ==============
    AIController();
    AIController(FoodManager *foodMgr);
    ~AIController() override;

    // ============== 实现接口方法 ==============
    Direction MakeDecision(const Snake &snake, const GameMap &map) override;
    void Update() override;
    void Init() override;
    const char *GetTypeName() const override { return "AIController"; }

private:
    /**
     * @brief 使用BFS寻找到最近食物的路径
     */
    bool FindPathToFood(const Snake &snake, const GameMap &map, const Point &foodPos);

    /**
     * @brief 使用A*寻路（可选实现）
     */
    bool FindPathAStar(const Snake &snake, const GameMap &map, const Point &foodPos);

    /**
     * @brief 评估某个方向的安全性（避免走进死路）
     */
    int EvaluateDirection(const Point &head, Direction dir, const Snake &snake, const GameMap &map);

    /**
     * @brief 选择最安全的方向（当无法到达食物时）
     */
    Direction ChooseSafeDirection(const Snake &snake, const GameMap &map);

    /**
     * @brief 寻找最近的食物
     */
    Point FindNearestFood(const Point &head);

    /**
     * @brief 判断某个位置是否安全
     */
    bool IsSafePosition(const Point &pos, const Snake &snake, const GameMap &map);
};
