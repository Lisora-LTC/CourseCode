#pragma once

// 全局常量、枚举、结构体、工具函数定义

// 定义 Windows 版本为 Vista 及以上，以支持 inet_pton 等 API
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

// 必须在包含 windows.h (graphics.h) 之前包含 winsock2.h
#include <winsock2.h>
#include <WS2tcpip.h> // inet_pton
#include <MSWSock.h>  // ConnectEx

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#include <cstdlib>
#include <ctime>
#include <vector>
#include <graphics.h> // EasyX 图形库

// ============== 常量定义 ==============
const int BLOCK_SIZE = 20;                         // 每个格子的像素大小
const int MAP_WIDTH = 40;                          // 地图宽度（格子数）
const int MAP_HEIGHT = 30;                         // 地图高度（格子数）
const int WINDOW_WIDTH = MAP_WIDTH * BLOCK_SIZE;   // 窗口宽度
const int WINDOW_HEIGHT = MAP_HEIGHT * BLOCK_SIZE; // 窗口高度

const int INITIAL_SNAKE_LENGTH = 3; // 蛇的初始长度
const int GAME_SPEED = 100;         // 游戏速度（毫秒/帧）

// ============== 结构体定义 ==============
struct Point
{
    int x; // X坐标（格子坐标，非像素）
    int y; // Y坐标

    Point() : x(0), y(0) {}
    Point(int _x, int _y) : x(_x), y(_y) {}

    // 重载比较运算符（用于容器查找）
    bool operator==(const Point &other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point &other) const
    {
        return !(*this == other);
    }

    // 用于std::map的排序（必须定义严格弱序）
    bool operator<(const Point &other) const
    {
        if (x != other.x)
            return x < other.x;
        return y < other.y;
    }
};

// ============== 枚举定义 ==============

// 方向枚举
enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};

// 游戏模式枚举
enum GameMode
{
    BEGINNER,  // 入门版：撞墙/撞身游戏结束
    ADVANCED,  // 进阶版：蛇尸变边界
    EXPERT,    // 高级版：蛇尸变食物
    SINGLE,    // 单人模式
    LOCAL_PVP, // 本地双人对战
    NET_PVP,   // 网络对战
    PVE,       // 人机对战
    EXIT       // 退出游戏
};

// 食物类型枚举
enum FoodType
{
    NORMAL_FOOD,  // 普通食物
    BONUS_FOOD,   // 加分食物
    MAGIC_FRUIT,  // 精灵果
    POISON_FRUIT, // 恶果
    SPEED_UP,     // 加速道具
    SPEED_DOWN    // 减速道具
};

// 游戏状态枚举
enum GameState
{
    MENU,      // 菜单界面
    PLAYING,   // 游戏中
    PAUSED,    // 暂停
    GAME_OVER, // 游戏结束
    SETTINGS   // 设置界面
};

// 墙壁类型枚举
enum WallType
{
    NO_WALL,   // 无墙
    HARD_WALL, // 硬墙（减半蛇长）
    SOFT_WALL, // 软墙（需吃2个食物补偿）
    BOUNDARY   // 边界墙
};

// ============== 工具函数类 ==============
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

    // ============== 窗口相关 ==============

    // 检查 EasyX 窗口是否被关闭
    static bool IsWindowClosed()
    {
        HWND hwnd = GetHWnd();
        if (hwnd == NULL || !IsWindow(hwnd))
        {
            return true;
        }
        return false;
    }

private:
    Utils() = delete; // 禁止实例化
};
