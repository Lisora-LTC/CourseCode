#include "KeyboardController.h"
#include "Snake.h"
#include "GameMap.h"
#include "Common.h"

// ============== 构造与析构 ==============
KeyboardController::KeyboardController() : playerIndex(0), lastDirection(NONE),
                                           bufferedInput(NONE), hasBufferedInput(false)
{
    InitKeyBindings(0);
    for (int i = 0; i < 4; i++)
        keyState[i] = false;
}

KeyboardController::KeyboardController(int player) : playerIndex(player), lastDirection(NONE),
                                                     bufferedInput(NONE), hasBufferedInput(false)
{
    InitKeyBindings(player);
    for (int i = 0; i < 4; i++)
        keyState[i] = false;
}

KeyboardController::~KeyboardController()
{
    // 无需特殊清理
}

// ============== 实现接口方法 ==============
Direction KeyboardController::MakeDecision(const Snake &snake, const GameMap &map)
{
    // 1. 获取蛇当前的方向
    Direction currentDir = snake.GetDirection();

    // 2. 如果有缓冲的输入，使用它
    Direction inputDir = NONE;
    if (hasBufferedInput)
    {
        inputDir = bufferedInput;
        hasBufferedInput = false; // 消费缓冲
        bufferedInput = NONE;

        // ✅ 记录最后的输入方向
        lastDirection = inputDir;
    }

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
    return inputDir;
}

void KeyboardController::CacheInput()
{
    // 使用边沿检测，只在按键刚按下时记录
    bool currentState[4];
    // 本地双人时P1只用WASD，P2用方向键
    currentState[0] = IsKeyPressed(keyUp);
    currentState[1] = IsKeyPressed(keyDown);
    currentState[2] = IsKeyPressed(keyLeft);
    currentState[3] = IsKeyPressed(keyRight);

    // 检测上键按下（边沿触发）
    if (currentState[0] && !keyState[0])
    {
        bufferedInput = UP;
        hasBufferedInput = true;
    }
    // 检测下键按下
    else if (currentState[1] && !keyState[1])
    {
        bufferedInput = DOWN;
        hasBufferedInput = true;
    }
    // 检测左键按下
    else if (currentState[2] && !keyState[2])
    {
        bufferedInput = LEFT;
        hasBufferedInput = true;
    }
    // 检测右键按下
    else if (currentState[3] && !keyState[3])
    {
        bufferedInput = RIGHT;
        hasBufferedInput = true;
    }

    // 更新按键状态
    for (int i = 0; i < 4; i++)
    {
        keyState[i] = currentState[i];
    }
}

void KeyboardController::Init()
{
    lastDirection = NONE;
}

// ============== 私有方法 ==============
Direction KeyboardController::DetectInput()
{
    // 按优先级检测按键（避免同时按多个键的冲突）
    // 本地双人时P1只用WASD，P2用方向键
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
        // 玩家1: WASD（但也支持方向键）
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
