#include "Snake.h"
#include "GameMap.h"
#include "Utils.h"

// ============== 构造与析构 ==============
Snake::Snake()
    : currentDirection(RIGHT), nextDirection(RIGHT), controller(nullptr),
      id(0), color(RGB(0, 255, 0)), isAlive(true)
{
    // 默认初始化一条蛇
    body.push_back(Point(5, 5));
    body.push_back(Point(4, 5));
    body.push_back(Point(3, 5));
}

Snake::Snake(int snakeId, const Point &startPos, Direction startDir, COLORREF snakeColor)
    : currentDirection(startDir), nextDirection(startDir), controller(nullptr),
      id(snakeId), color(snakeColor), isAlive(true)
{
    // TODO: 根据起始位置和方向初始化蛇身
}

Snake::~Snake()
{
    // 注意：不delete controller，因为controller的生命周期由外部管理
    controller = nullptr;
}

// ============== 控制器管理 ==============
void Snake::SetController(IController *ctrl)
{
    // TODO: 设置控制器
}

// ============== 核心逻辑 ==============
void Snake::Update(GameMap &map)
{
    // TODO: 向控制器询问方向，然后移动
    // 1. 调用 controller->MakeDecision(*this, map)
    // 2. 根据决策结果更新方向
    // 3. 调用 Move()
}

void Snake::Move()
{
    // TODO: 实现蛇的移动逻辑
    // 1. 根据 currentDirection 计算新的头部位置
    // 2. 在body前端插入新头部
    // 3. 删除body末端
}

void Snake::Grow()
{
    // TODO: 实现蛇的生长逻辑
    // 不删除尾部，只添加头部
}

void Snake::ChangeDirection(Direction newDir)
{
    // TODO: 实现方向改变逻辑
    // 注意：不能直接180度转弯
}

// ============== 碰撞检测 ==============
bool Snake::CheckSelfCollision() const
{
    // TODO: 检测蛇头是否撞到蛇身
    return false;
}

bool Snake::CheckCollisionWith(const Snake &other) const
{
    // TODO: 检测是否撞到另一条蛇
    return false;
}

// ============== 工具方法 ==============
void Snake::Reset(const Point &startPos, Direction startDir)
{
    // TODO: 重置蛇到初始状态
}

void Snake::ShrinkByHalf()
{
    // TODO: 减半蛇身长度（撞硬墙惩罚）
}
