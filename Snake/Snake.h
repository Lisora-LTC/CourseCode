#pragma once
#include "Common.h"
#include "IController.h"
#include <vector>
#include <graphics.h> // 用于颜色定义

// 前向声明
class GameMap;

// ============== 蛇类 ==============
class Snake
{
private:
    std::vector<Point> body;    // 蛇身（body[0]是蛇头）
    Direction currentDirection; // 当前移动方向
    Direction nextDirection;    // 下一帧的方向（缓冲，防止快速连按）
    IController *controller;    // 控制器指针（核心：解耦控制逻辑）

    int id;         // 蛇的ID（用于多蛇区分）
    COLORREF color; // 蛇的颜色
    bool isAlive;   // 是否存活

public:
    // ============== 构造与析构 ==============
    Snake();
    Snake(int snakeId, const Point &startPos, Direction startDir, COLORREF snakeColor);
    ~Snake();

    // ============== 控制器管理 ==============
    void SetController(IController *ctrl);
    IController *GetController() const { return controller; }

    // ============== 核心逻辑 ==============
    /**
     * @brief 每帧更新 - 向控制器询问方向，然后移动
     * @param map 地图引用
     */
    void Update(GameMap &map);

    /**
     * @brief 移动蛇（不吃食物时）
     */
    void Move();

    /**
     * @brief 生长（吃到食物时）
     */
    void Grow();

    /**
     * @brief 改变方向
     * @param newDir 新方向
     */
    void ChangeDirection(Direction newDir);

    // ============== 碰撞检测 ==============
    /**
     * @brief 检测是否撞到自己
     */
    bool CheckSelfCollision() const;

    /**
     * @brief 检测是否撞到另一条蛇
     */
    bool CheckCollisionWith(const Snake &other) const;

    // ============== Getter 方法 ==============
    const std::vector<Point> &GetBody() const { return body; }
    Point GetHead() const { return body.empty() ? Point() : body.front(); }
    Direction GetDirection() const { return currentDirection; }
    int GetLength() const { return static_cast<int>(body.size()); }
    int GetId() const { return id; }
    COLORREF GetColor() const { return color; }
    bool IsAlive() const { return isAlive; }

    // ============== Setter 方法 ==============
    void SetAlive(bool alive) { isAlive = alive; }
    void SetColor(COLORREF newColor) { color = newColor; }

    // ============== 工具方法 ==============
    /**
     * @brief 重置蛇到初始状态
     */
    void Reset(const Point &startPos, Direction startDir);

    /**
     * @brief 减半蛇身（撞硬墙惩罚）
     */
    void ShrinkByHalf();
};
