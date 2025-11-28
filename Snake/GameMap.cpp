#include "GameMap.h"

// ============== 构造与析构 ==============
GameMap::GameMap() : width(MAP_WIDTH), height(MAP_HEIGHT)
{
    // 初始化二维墙壁数组
    walls.resize(height, std::vector<WallType>(width, NO_WALL));

    // 初始化边界墙
    InitBoundaryWalls();
}

GameMap::GameMap(int w, int h) : width(w), height(h)
{
    // 初始化指定大小的地图
    walls.resize(height, std::vector<WallType>(width, NO_WALL));

    // 初始化边界墙
    InitBoundaryWalls();
}

GameMap::~GameMap()
{
    walls.clear();
    obstacles.clear();
}

// ============== 墙壁管理 ==============
void GameMap::InitBoundaryWalls()
{
    // 遍历边界，设置为BOUNDARY类型
    for (int x = 0; x < width; x++)
    {
        walls[0][x] = BOUNDARY;          // 上边界
        walls[height - 1][x] = BOUNDARY; // 下边界
        obstacles.insert(Point(x, 0));
        obstacles.insert(Point(x, height - 1));
    }

    for (int y = 0; y < height; y++)
    {
        walls[y][0] = BOUNDARY;         // 左边界
        walls[y][width - 1] = BOUNDARY; // 右边界
        obstacles.insert(Point(0, y));
        obstacles.insert(Point(width - 1, y));
    }
}

void GameMap::AddWall(const Point &pos, WallType type)
{
    if (!IsInBounds(pos))
        return;

    walls[pos.y][pos.x] = type;
    if (type != NO_WALL)
    {
        obstacles.insert(pos);
    }
}

void GameMap::RemoveWall(const Point &pos)
{
    if (!IsInBounds(pos))
        return;

    walls[pos.y][pos.x] = NO_WALL;
    obstacles.erase(pos);
}

void GameMap::ClearWalls()
{
    // 清除所有非边界墙壁
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            if (walls[y][x] != NO_WALL)
            {
                walls[y][x] = NO_WALL;
                obstacles.erase(Point(x, y));
            }
        }
    }
}

// ============== 查询方法 ==============
bool GameMap::IsWall(const Point &p) const
{
    if (!IsInBounds(p))
        return true;
    return walls[p.y][p.x] != NO_WALL;
}

WallType GameMap::GetWallType(const Point &p) const
{
    if (!IsInBounds(p))
        return BOUNDARY;
    return walls[p.y][p.x];
}

bool GameMap::IsInBounds(const Point &p) const
{
    return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
}

bool GameMap::IsWalkable(const Point &p) const
{
    return IsInBounds(p) && !IsWall(p);
}

// ============== 特殊功能 ==============
void GameMap::ConvertSnakeToWalls(const std::vector<Point> &snakeBody)
{
    // 将蛇尸转换为软墙（进阶版功能）
    for (const auto &segment : snakeBody)
    {
        if (IsInBounds(segment) && !IsWall(segment))
        {
            AddWall(segment, SOFT_WALL);
        }
    }
}

std::vector<Point> GameMap::GetEmptyPositions() const
{
    std::vector<Point> emptyPositions;

    // 遍历地图，找出所有空闲位置
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            Point p(x, y);
            if (!IsWall(p))
            {
                emptyPositions.push_back(p);
            }
        }
    }

    return emptyPositions;
}
