#pragma once
#include "Common.h"
#include <cstdlib>
#include <ctime>
#include <vector>

// 工具函数类（纯静态）
class Utils
{
public:
    // ============== 随机数相关 ==============

    // 初始化随机数种子
    static void InitRandom()
    {
        srand(static_cast<unsigned int>(time(nullptr)));
    }

    // 生成 [min, max] 范围内的随机整数
    static int RandomInt(int min, int max)
    {
        if (min > max)
            return min;
        return min + rand() % (max - min + 1);
    }

    // 生成随机点（在地图范围内）
    static Point RandomPoint()
    {
        return Point(RandomInt(0, MAP_WIDTH - 1), RandomInt(0, MAP_HEIGHT - 1));
    }

    // ============== 碰撞检测相关 ==============

    // 检查点是否在边界内
    static bool IsInBounds(const Point &p)
    {
        return p.x >= 0 && p.x < MAP_WIDTH && p.y >= 0 && p.y < MAP_HEIGHT;
    }

    // 检查两点是否重合
    static bool IsCollision(const Point &p1, const Point &p2)
    {
        return p1.x == p2.x && p1.y == p2.y;
    }

    // 检查点是否在点列表中（用于检测蛇身碰撞）
    static bool IsPointInList(const Point &p, const std::vector<Point> &list)
    {
        for (const auto &point : list)
        {
            if (IsCollision(p, point))
            {
                return true;
            }
        }
        return false;
    }

    // ============== 方向相关 ==============

    // 根据方向获取下一个点
    static Point GetNextPoint(const Point &current, Direction dir)
    {
        Point next = current;
        switch (dir)
        {
        case UP:
            next.y--;
            break;
        case DOWN:
            next.y++;
            break;
        case LEFT:
            next.x--;
            break;
        case RIGHT:
            next.x++;
            break;
        case NONE:
            break;
        }
        return next;
    }

    // 判断两个方向是否相反
    static bool IsOppositeDirection(Direction dir1, Direction dir2)
    {
        return (dir1 == UP && dir2 == DOWN) ||
               (dir1 == DOWN && dir2 == UP) ||
               (dir1 == LEFT && dir2 == RIGHT) ||
               (dir1 == RIGHT && dir2 == LEFT);
    }

    // ============== 距离计算 ==============

    // 曼哈顿距离
    static int ManhattanDistance(const Point &p1, const Point &p2)
    {
        return abs(p1.x - p2.x) + abs(p1.y - p2.y);
    }

    // 欧几里得距离的平方（避免开方运算）
    static int DistanceSquared(const Point &p1, const Point &p2)
    {
        int dx = p1.x - p2.x;
        int dy = p1.y - p2.y;
        return dx * dx + dy * dy;
    }

private:
    Utils() = delete; // 禁止实例化
};
