#include "KeyboardController.h"
#include "Snake.h"
#include "GameMap.h"
#include "Common.h"

// ============== 构造与析构 ==============
KeyboardController::KeyboardController() : playerIndex(0), lastDirection(NONE)
{
    InitKeyBindings(0);
}

KeyboardController::KeyboardController(int player) : playerIndex(player), lastDirection(NONE)
{
    InitKeyBindings(player);
}

KeyboardController::~KeyboardController()
{
    // 无需特殊清理
}

// ============== 实现接口方法 ==============
Direction KeyboardController::MakeDecision(const Snake &snake, const GameMap &map)
{
    // 1. 检测当前键盘输入
    Direction inputDir = DetectInput();

    // 2. 获取蛇当前的方向
    Direction currentDir = snake.GetDirection();

    // 3. 如果没有输入，保持当前方向
    if (inputDir == NONE)
    {
        lastDirection = currentDir;
        return currentDir;
    }

    // 4. 检查是否与当前方向相反（不允许180度转向）
    if (Utils::IsOppositeDirection(inputDir, currentDir))
    {
        // 保持原方向
        return currentDir;
    }

    // 5. 返回有效的新方向
    lastDirection = inputDir;
    return inputDir;
}

void KeyboardController::Init()
{
    lastDirection = NONE;
}

// ============== 私有方法 ==============
Direction KeyboardController::DetectInput()
{
    // 按优先级检测按键（避免同时按多个键的冲突）
    if (IsKeyPressed(keyUp))
        return UP;
    if (IsKeyPressed(keyDown))
        return DOWN;
    if (IsKeyPressed(keyLeft))
        return LEFT;
    if (IsKeyPressed(keyRight))
        return RIGHT;

    return NONE;
}

void KeyboardController::InitKeyBindings(int player)
{
    if (player == 0)
    {
        // 玩家1: WASD
        keyUp = 'W';
        keyDown = 'S';
        keyLeft = 'A';
        keyRight = 'D';
    }
    else
    {
        // 玩家2: 方向键
        keyUp = VK_UP;
        keyDown = VK_DOWN;
        keyLeft = VK_LEFT;
        keyRight = VK_RIGHT;
    }
}

bool KeyboardController::IsKeyPressed(int vkCode)
{
    // GetAsyncKeyState 返回值的最高位表示按键是否按下
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}
