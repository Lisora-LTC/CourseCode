#include "Snake.h"
#include "GameMap.h"

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
    // 根据起始位置和方向初始化蛇身
    body.clear();
    body.push_back(startPos);

    // 根据方向添加蛇身
    for (int i = 1; i < INITIAL_SNAKE_LENGTH; i++)
    {
        Point newPoint = startPos;
        switch (startDir)
        {
        case UP:
            newPoint.y += i;
            break;
        case DOWN:
            newPoint.y -= i;
            break;
        case LEFT:
            newPoint.x += i;
            break;
        case RIGHT:
            newPoint.x -= i;
            break;
        default:
            break;
        }
        body.push_back(newPoint);
    }
}

Snake::~Snake()
{
    // 注意：不delete controller，因为controller的生命周期由外部管理
    controller = nullptr;
}

// ============== 控制器管理 ==============
void Snake::SetController(IController *ctrl)
{
    controller = ctrl;
    if (controller)
    {
        controller->Init();
    }
}

// ============== 核心逻辑 ==============
void Snake::Update(GameMap &map)
{
    if (!isAlive || !controller)
        return;

    // 向控制器询问方向
    Direction newDir = controller->MakeDecision(*this, map);

    // 更新方向（检查是否有效）
    if (newDir != NONE)
    {
        ChangeDirection(newDir);
    }

    // 应用缓冲的方向
    currentDirection = nextDirection;

    // 移动蛇
    Move();
}

void Snake::Move()
{
    if (body.empty())
        return;

    // 计算新的头部位置
    Point newHead = Utils::GetNextPoint(body.front(), currentDirection);

    // 在前端插入新头部
    body.insert(body.begin(), newHead);

    // 删除尾部
    body.pop_back();
}

void Snake::Grow()
{
    if (body.empty())
        return;

    // 计算新的头部位置
    Point newHead = Utils::GetNextPoint(body.front(), currentDirection);

    // 在前端插入新头部（不删除尾部）
    body.insert(body.begin(), newHead);
}

void Snake::ChangeDirection(Direction newDir)
{
    // 不能直接180度转弯
    if (Utils::IsOppositeDirection(currentDirection, newDir))
    {
        return;
    }

    // 不能是无效方向
    if (newDir == NONE)
    {
        return;
    }

    // 缓冲新方向（下一帧应用）
    nextDirection = newDir;
}

// ============== 碰撞检测 ==============
bool Snake::CheckSelfCollision() const
{
    if (body.size() < 2)
        return false;

    const Point &head = body.front();

    // 检查头部是否撞到身体（从第4节开始检查，前3节不可能撞到）
    for (size_t i = 4; i < body.size(); i++)
    {
        if (Utils::IsCollision(head, body[i]))
        {
            return true;
        }
    }

    return false;
}

bool Snake::CheckCollisionWith(const Snake &other) const
{
    if (body.empty() || other.body.empty())
        return false;

    const Point &myHead = body.front();

    // 检查我的头是否撞到对方的身体
    for (const auto &segment : other.body)
    {
        if (Utils::IsCollision(myHead, segment))
        {
            return true;
        }
    }

    return false;
}

// ============== 工具方法 ==============
void Snake::Reset(const Point &startPos, Direction startDir)
{
    body.clear();
    currentDirection = startDir;
    nextDirection = startDir;
    isAlive = true;

    // 重新初始化蛇身
    body.push_back(startPos);
    for (int i = 1; i < INITIAL_SNAKE_LENGTH; i++)
    {
        Point newPoint = startPos;
        switch (startDir)
        {
        case UP:
            newPoint.y += i;
            break;
        case DOWN:
            newPoint.y -= i;
            break;
        case LEFT:
            newPoint.x += i;
            break;
        case RIGHT:
            newPoint.x -= i;
            break;
        default:
            break;
        }
        body.push_back(newPoint);
    }
}

void Snake::ShrinkByHalf()
{
    if (body.size() <= 2)
    {
        // 至少保留2节
        while (body.size() > 2)
        {
            body.pop_back();
        }
        return;
    }

    size_t newSize = body.size() / 2;
    if (newSize < 2)
        newSize = 2;

    body.resize(newSize);
}
