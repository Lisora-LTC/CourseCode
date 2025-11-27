#include "KeyboardController.h"
#include "Snake.h"
#include "GameMap.h"

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
    // TODO: 清理
}

// ============== 实现接口方法 ==============
Direction KeyboardController::MakeDecision(const Snake &snake, const GameMap &map)
{
    // TODO: 检测键盘输入并返回方向
    // 1. 调用 DetectInput() 获取输入
    // 2. 检查是否与当前方向相反（不允许180度转向）
    // 3. 返回有效方向

    return snake.GetDirection(); // 临时返回当前方向
}

void KeyboardController::Init()
{
    // TODO: 初始化控制器
    lastDirection = NONE;
}

// ============== 私有方法 ==============
Direction KeyboardController::DetectInput()
{
    // TODO: 使用 GetAsyncKeyState 检测按键
    // 检查 keyUp, keyDown, keyLeft, keyRight

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
    // TODO: 根据玩家编号初始化键位
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
    // TODO: 使用 GetAsyncKeyState 检测按键状态
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}
