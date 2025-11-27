#include "GameMap.h"
#include "Utils.h"

// ============== 构造与析构 ==============
GameMap::GameMap() : width(MAP_WIDTH), height(MAP_HEIGHT)
{
    // TODO: 初始化地图
}

GameMap::GameMap(int w, int h) : width(w), height(h)
{
    // TODO: 初始化指定大小的地图
    // 初始化walls二维数组
}

GameMap::~GameMap()
{
    // TODO: 清理资源
}

// ============== 墙壁管理 ==============
void GameMap::InitBoundaryWalls()
{
    // TODO: 初始化边界墙
    // 遍历边界，设置为BOUNDARY类型
}

void GameMap::AddWall(const Point &pos, WallType type)
{
    // TODO: 添加墙壁
}

void GameMap::RemoveWall(const Point &pos)
{
    // TODO: 移除墙壁
}

void GameMap::ClearWalls()
{
    // TODO: 清除所有墙壁（除边界外）
}

// ============== 查询方法 ==============
bool GameMap::IsWall(const Point &p) const
{
    // TODO: 判断是否是墙
    return false;
}

WallType GameMap::GetWallType(const Point &p) const
{
    // TODO: 获取墙壁类型
    return NO_WALL;
}

bool GameMap::IsInBounds(const Point &p) const
{
    // TODO: 判断是否在边界内
    return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
}

bool GameMap::IsWalkable(const Point &p) const
{
    // TODO: 判断位置是否可通行（不是墙且在边界内）
    return false;
}

// ============== 特殊功能 ==============
void GameMap::ConvertSnakeToWalls(const std::vector<Point> &snakeBody)
{
    // TODO: 将蛇尸转换为墙壁（进阶版功能）
}

std::vector<Point> GameMap::GetEmptyPositions() const
{
    // TODO: 获取所有空闲位置
    std::vector<Point> emptyPositions;
    return emptyPositions;
}
