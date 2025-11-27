#pragma once
#include "Common.h"
#include <vector>
#include <set>

// ============== 地图类 ==============
class GameMap
{
private:
    int width;
    int height;
    std::vector<std::vector<WallType>> walls; // 二维数组存储墙壁信息
    std::set<Point> obstacles;                // 障碍物位置集合（用于快速查找）

public:
    // ============== 构造与析构 ==============
    GameMap();
    GameMap(int w, int h);
    ~GameMap();

    // ============== 墙壁管理 ==============
    /**
     * @brief 初始化边界墙
     */
    void InitBoundaryWalls();

    /**
     * @brief 添加墙壁
     */
    void AddWall(const Point &pos, WallType type);

    /**
     * @brief 移除墙壁
     */
    void RemoveWall(const Point &pos);

    /**
     * @brief 清除所有墙壁（除边界外）
     */
    void ClearWalls();

    // ============== 查询方法 ==============
    /**
     * @brief 判断某个位置是否是墙
     */
    bool IsWall(const Point &p) const;

    /**
     * @brief 获取墙壁类型
     */
    WallType GetWallType(const Point &p) const;

    /**
     * @brief 判断点是否在地图内
     */
    bool IsInBounds(const Point &p) const;

    /**
     * @brief 判断位置是否可通行
     */
    bool IsWalkable(const Point &p) const;

    // ============== Getter 方法 ==============
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    const std::vector<std::vector<WallType>> &GetWalls() const { return walls; }

    // ============== 特殊功能（进阶版/高级版） ==============
    /**
     * @brief 将蛇尸转换为墙壁（进阶版）
     */
    void ConvertSnakeToWalls(const std::vector<Point> &snakeBody);

    /**
     * @brief 获取所有空闲位置（用于生成食物）
     */
    std::vector<Point> GetEmptyPositions() const;
};

// 比较运算符重载（用于 set 存储 Point）
inline bool operator<(const Point &a, const Point &b)
{
    if (a.x != b.x)
        return a.x < b.x;
    return a.y < b.y;
}
