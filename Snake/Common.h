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
// 游戏区域配置（3/4屏幕面积，36px大格子）
const int BLOCK_SIZE = 36;                            // 每个格子的像素大小 (36px)
const int MAP_WIDTH = 38;                             // 地图宽度（格子数）38格
const int MAP_HEIGHT = 28;                            // 地图高度（格子数）28格
const int GAME_AREA_WIDTH = MAP_WIDTH * BLOCK_SIZE;   // 游戏区域宽度 (1368px)
const int GAME_AREA_HEIGHT = MAP_HEIGHT * BLOCK_SIZE; // 游戏区域高度 (1008px)

// 窗口布局配置（1920x1080全屏）
const int GAME_AREA_X = 36;        // 游戏区左边距 (36px，留一个格子空隙)
const int GAME_AREA_Y = 36;        // 游戏区上边距 (36px，留一个格子空隙)
const int SIDEBAR_X = 1440;        // 侧边栏X坐标 (屏幕右侧1/4)
const int SIDEBAR_Y = 0;           // 侧边栏Y坐标 (从顶部开始)
const int SIDEBAR_WIDTH = 480;     // 侧边栏宽度 (480px，占屏幕1/4)
const int SIDEBAR_HEIGHT = 1080;   // 侧边栏高度 (全屏高)
const int SIDEBAR_CENTER_X = 1680; // 侧边栏中轴线 (1440 + 240)
const int WINDOW_WIDTH = 1920;     // 窗口总宽度 (1920px)
const int WINDOW_HEIGHT = 1080;    // 窗口总高度 (1080px)

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

// ============== 网络房间相关数据结构 ==============

// 房间状态枚举
enum RoomStatus
{
    ROOM_WAITING, // 等待中（可加入）
    ROOM_PLAYING, // 游戏中（不可加入）
    ROOM_FULL     // 已满员（不可加入）
};

// 玩家状态枚举
enum PlayerStatus
{
    PLAYER_NOT_READY, // 未准备
    PLAYER_READY,     // 已准备
    PLAYER_OFFLINE    // 离线
};

// 房间信息结构体
struct RoomInfo
{
    uint32_t roomId;      // 房间ID（唯一标识符）
    wchar_t roomName[40]; // 房间名称
    wchar_t hostName[40]; // 房主昵称
    RoomStatus status;    // 房间状态
    int currentPlayers;   // 当前人数
    int maxPlayers;       // 最大人数（默认2）

    RoomInfo()
        : roomId(0), status(ROOM_WAITING), currentPlayers(0), maxPlayers(2)
    {
        wmemset(roomName, 0, sizeof(roomName) / sizeof(wchar_t));
        wmemset(hostName, 0, sizeof(hostName) / sizeof(wchar_t));
    }
};

// 大厅玩家信息结构体
struct LobbyPlayerInfo
{
    wchar_t nickname[40]; // 玩家昵称
    PlayerStatus status;  // 玩家状态
    COLORREF avatarColor; // 头像颜色
    bool isHost;          // 是否为房主

    LobbyPlayerInfo()
        : status(PLAYER_NOT_READY),
          avatarColor(RGB(63, 114, 175)), // 默认亮蓝色
          isHost(false)
    {
        wmemset(nickname, 0, sizeof(nickname) / sizeof(wchar_t));
    }

    LobbyPlayerInfo(const wchar_t *name, PlayerStatus s, COLORREF color, bool host = false)
        : status(s), avatarColor(color), isHost(host)
    {
        wcsncpy_s(nickname, name, sizeof(nickname) / sizeof(wchar_t) - 1);
    }
};

// 大厅状态数据包（用于同步大厅状态）
struct LobbyUpdatePacket
{
    uint32_t roomId;         // 房间ID
    LobbyPlayerInfo player1; // 玩家1（房主）
    LobbyPlayerInfo player2; // 玩家2（客人）
    bool player2Connected;   // 玩家2是否已连接
    bool bothReady;          // 双方是否都准备好
    char messageType;        // 消息类型 ('J'=加入, 'L'=离开, 'R'=准备, 'S'=开始游戏)

    LobbyUpdatePacket()
        : roomId(0), player2Connected(false), bothReady(false), messageType('U') {}
};
