#include "FoodManager.h"
#include "Utils.h"

// ============== 构造与析构 ==============
FoodManager::FoodManager() : minFoodCount(1), maxFoodCount(5)
{
    // TODO: 初始化
}

FoodManager::FoodManager(int minCount, int maxCount)
    : minFoodCount(minCount), maxFoodCount(maxCount)
{
    // TODO: 初始化
}

FoodManager::~FoodManager()
{
    // TODO: 清理
}

// ============== 食物生成 ==============
void FoodManager::SpawnFood(const Snake &snake, const GameMap &map)
{
    // TODO: 生成随机数量的食物（1-5个）
}

void FoodManager::SpawnFoodCount(int count, const Snake &snake, const GameMap &map)
{
    // TODO: 生成指定数量的食物
}

void FoodManager::SpawnFoodAt(const Point &pos, FoodType type)
{
    // TODO: 在指定位置生成食物
}

Food FoodManager::GenerateRandomFood(const Point &pos)
{
    // TODO: 生成随机类型的食物
    // 普通食物概率最高，特殊食物概率较低
    return Food(pos, NORMAL_FOOD, 10);
}

// ============== 食物检测与消费 ==============
bool FoodManager::HasFoodAt(const Point &p) const
{
    // TODO: 检测指定位置是否有食物
    return false;
}

Food *FoodManager::GetFoodAt(const Point &p)
{
    // TODO: 获取指定位置的食物
    return nullptr;
}

int FoodManager::ConsumeFood(const Point &p)
{
    // TODO: 吃掉食物，返回得分
    return 0;
}

void FoodManager::RemoveFoodAt(const Point &p)
{
    // TODO: 移除指定位置的食物
}

// ============== 工具方法 ==============
void FoodManager::ClearAllFoods()
{
    // TODO: 清空所有食物
    foods.clear();
}

bool FoodManager::NeedMoreFood() const
{
    // TODO: 判断是否需要补充食物
    return foods.empty();
}

void FoodManager::Update()
{
    // TODO: 更新食物状态（例如特殊食物的闪烁效果）
}

// ============== 私有辅助方法 ==============
Point FoodManager::FindEmptyPosition(const Snake &snake, const GameMap &map) const
{
    // TODO: 找到一个可用的空位置
    // 循环尝试随机位置，直到找到空位
    return Point();
}

bool FoodManager::IsPositionOccupied(const Point &p, const Snake &snake, const GameMap &map) const
{
    // TODO: 判断位置是否被占用
    // 检查是否在蛇身上、墙上或已有食物上
    return false;
}
