#pragma once
#include "Common.h"
#include "Snake.h"
#include "GameMap.h"
#include <vector>

// ============== 食物信息结构 ==============
struct Food
{
    Point position;
    FoodType type;
    int scoreValue; // 得分值

    Food() : position(), type(NORMAL_FOOD), scoreValue(10) {}
    Food(const Point &pos, FoodType t, int score)
        : position(pos), type(t), scoreValue(score) {}
};

// ============== 食物管理类 ==============
class FoodManager
{
private:
    std::vector<Food> foods; // 当前地图上的所有食物
    int minFoodCount;        // 最少食物数量
    int maxFoodCount;        // 最多食物数量

public:
    // ============== 构造与析构 ==============
    FoodManager();
    FoodManager(int minCount, int maxCount);
    ~FoodManager();

    // ============== 食物生成 ==============
    /**
     * @brief 生成随机数量的食物
     * @param snake 蛇的引用（避开蛇身）
     * @param map 地图引用（避开墙壁）
     */
    void SpawnFood(const Snake &snake, const GameMap &map);

    /**
     * @brief 生成指定数量的食物
     */
    void SpawnFoodCount(int count, const Snake &snake, const GameMap &map);

    /**
     * @brief 在指定位置生成食物
     */
    void SpawnFoodAt(const Point &pos, FoodType type);

    /**
     * @brief 生成随机类型的食物
     */
    Food GenerateRandomFood(const Point &pos);

    // ============== 食物检测与消费 ==============
    /**
     * @brief 检测指定位置是否有食物
     */
    bool HasFoodAt(const Point &p) const;

    /**
     * @brief 获取指定位置的食物
     */
    Food *GetFoodAt(const Point &p);

    /**
     * @brief 吃掉指定位置的食物（返回得分）
     */
    int ConsumeFood(const Point &p);

    /**
     * @brief 移除指定位置的食物
     */
    void RemoveFoodAt(const Point &p);

    // ============== Getter 方法 ==============
    const std::vector<Food> &GetAllFoods() const { return foods; }
    int GetFoodCount() const { return static_cast<int>(foods.size()); }

    // ============== 工具方法 ==============
    /**
     * @brief 清空所有食物
     */
    void ClearAllFoods();

    /**
     * @brief 检查是否需要补充食物
     */
    bool NeedMoreFood() const;

    /**
     * @brief 更新食物状态（用于特殊食物效果）
     */
    void Update();

private:
    /**
     * @brief 找到一个可用的空位置
     */
    Point FindEmptyPosition(const Snake &snake, const GameMap &map) const;

    /**
     * @brief 判断位置是否被占用
     */
    bool IsPositionOccupied(const Point &p, const Snake &snake, const GameMap &map) const;
};
