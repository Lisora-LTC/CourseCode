#pragma once

// 全局常量、枚举、结构体定义

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
    PVE        // 人机对战
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
