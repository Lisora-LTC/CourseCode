#include "FoodManager.h"

// ============== 构造与析构 ==============
FoodManager::FoodManager() : minFoodCount(1), maxFoodCount(5)
{
    Utils::InitRandom();
}

FoodManager::FoodManager(int minCount, int maxCount)
    : minFoodCount(minCount), maxFoodCount(maxCount)
{
    Utils::InitRandom();
}

FoodManager::~FoodManager()
{
    foods.clear();
}

// ============== 食物生成 ==============
void FoodManager::SpawnFood(const Snake &snake, const GameMap &map)
{
    // 生成随机数量的食物（1-5个）
    int count = Utils::RandomInt(minFoodCount, maxFoodCount);
    SpawnFoodCount(count, snake, map);
}

void FoodManager::SpawnFoodCount(int count, const Snake &snake, const GameMap &map)
{
    for (int i = 0; i < count; i++)
    {
        Point pos = FindEmptyPosition(snake, map);
        if (pos.x >= 0 && pos.y >= 0)
        { // 找到有效位置
            Food food = GenerateRandomFood(pos);
            foods.push_back(food);
        }
    }
}

void FoodManager::SpawnFoodAt(const Point &pos, FoodType type)
{
    int score = 10; // 默认分数
    switch (type)
    {
    case NORMAL_FOOD:
        score = 10;
        break;
    case BONUS_FOOD:
        score = 20;
        break;
    case MAGIC_FRUIT:
        score = 50;
        break;
    case POISON_FRUIT:
        score = -100;
        break;
    case SPEED_UP:
        score = 5;
        break;
    case SPEED_DOWN:
        score = 5;
        break;
    }
    foods.push_back(Food(pos, type, score));
}

Food FoodManager::GenerateRandomFood(const Point &pos)
{
    // 只生成普通食物(90%)和加速食物(10%)
    int rand = Utils::RandomInt(1, 100);

    FoodType type;
    int score;

    if (rand <= 90)
    {
        // 90% 概率生成普通食物
        type = NORMAL_FOOD;
        score = 10;
    }
    else
    {
        // 10% 概率生成加速食物
        type = SPEED_UP;
        score = 5;
    }

    return Food(pos, type, score);
}

// ============== 食物检测与消费 ==============
bool FoodManager::HasFoodAt(const Point &p) const
{
    for (const auto &food : foods)
    {
        if (Utils::IsCollision(food.position, p))
        {
            return true;
        }
    }
    return false;
}

Food *FoodManager::GetFoodAt(const Point &p)
{
    for (auto &food : foods)
    {
        if (Utils::IsCollision(food.position, p))
        {
            return &food;
        }
    }
    return nullptr;
}

int FoodManager::ConsumeFood(const Point &p)
{
    for (auto it = foods.begin(); it != foods.end(); ++it)
    {
        if (Utils::IsCollision(it->position, p))
        {
            int score = it->scoreValue;
            foods.erase(it);
            return score; // 立即返回，避免迭代器失效
        }
    }
    return 0; // 没有找到食物
}

void FoodManager::RemoveFoodAt(const Point &p)
{
    for (auto it = foods.begin(); it != foods.end(); ++it)
    {
        if (Utils::IsCollision(it->position, p))
        {
            foods.erase(it);
            return;
        }
    }
}

// ============== 工具方法 ==============
void FoodManager::ClearAllFoods()
{
    foods.clear();
}

bool FoodManager::NeedMoreFood() const
{
    // 当食物数量<=1时就刷新，避免地图上完全没有食物
    return foods.size() <= 1;
}

void FoodManager::Update()
{
    // 更新食物状态（例如特殊食物的闪烁效果）
    // 目前不需要实现
}

// ============== 私有辅助方法 ==============
Point FoodManager::FindEmptyPosition(const Snake &snake, const GameMap &map) const
{
    // 找到一个可用的空位置（避免在最靠近墙边的一圈生成）
    int maxAttempts = 100; // 最多尝试100次

    for (int i = 0; i < maxAttempts; i++)
    {
        // ✅ 在距离墙边至少2格的区域随机生成
        int x = Utils::RandomInt(2, MAP_WIDTH - 3);
        int y = Utils::RandomInt(2, MAP_HEIGHT - 3);
        Point pos(x, y);

        if (!IsPositionOccupied(pos, snake, map))
        {
            return pos;
        }
    }

    // 如果随机失败，遍历所有位置（排除最靠近墙边的一圈）
    for (int y = 2; y < MAP_HEIGHT - 2; y++)
    {
        for (int x = 2; x < MAP_WIDTH - 2; x++)
        {
            Point pos(x, y);
            if (!IsPositionOccupied(pos, snake, map))
            {
                return pos;
            }
        }
    }

    return Point(-1, -1); // 没有可用位置
}

bool FoodManager::IsPositionOccupied(const Point &p, const Snake &snake, const GameMap &map) const
{
    // 检查是否在墙上
    if (map.IsWall(p))
    {
        return true;
    }

    // 检查是否在蛇身上
    if (Utils::IsPointInList(p, snake.GetBody()))
    {
        return true;
    }

    // 检查是否已有食物
    if (HasFoodAt(p))
    {
        return true;
    }

    return false;
}
