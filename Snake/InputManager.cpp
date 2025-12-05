#include "InputManager.h"

// ============== 构造与析构 ==============
InputManager::InputManager()
    : leftButtonLastState(false), rightButtonLastState(false)
{
}

InputManager::~InputManager()
{
}

// ============== 键盘输入 ==============
bool InputManager::IsKeyJustPressed(int vkCode)
{
    bool currentState = GetKeyState(vkCode);
    bool lastState = GetKeyLastState(vkCode);

    // 边沿触发：当前按下 && 上一帧未按下
    bool justPressed = currentState && !lastState;

    // 更新状态
    SetKeyLastState(vkCode, currentState);

    return justPressed;
}

bool InputManager::IsKeyHeld(int vkCode)
{
    return GetKeyState(vkCode);
}

// ============== 鼠标输入 ==============
bool InputManager::GetLatestMouseMessage(MOUSEMSG &msg)
{
    // 清空队列，只保留最新消息
    MOUSEMSG latestMsg;
    bool hasMessage = false;

    while (MouseHit())
    {
        latestMsg = GetMouseMsg();
        hasMessage = true;
    }

    if (hasMessage)
    {
        msg = latestMsg;
        return true;
    }

    return false;
}

bool InputManager::IsLeftButtonJustPressed()
{
    bool currentState = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    bool justPressed = currentState && !leftButtonLastState;
    leftButtonLastState = currentState;
    return justPressed;
}

bool InputManager::IsRightButtonJustPressed()
{
    bool currentState = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    bool justPressed = currentState && !rightButtonLastState;
    rightButtonLastState = currentState;
    return justPressed;
}

void InputManager::ClearMouseQueue()
{
    while (MouseHit())
    {
        GetMouseMsg();
    }
}

// ============== 窗口相关 ==============
bool InputManager::IsWindowClosed()
{
    HWND hwnd = GetHWnd();
    if (hwnd == NULL || !IsWindow(hwnd))
    {
        return true;
    }
    return false;
}

// ============== 状态更新 ==============
void InputManager::Update()
{
    // 更新所有已追踪按键的状态
    for (auto &pair : keyLastState)
    {
        int vkCode = pair.first;
        pair.second = GetKeyState(vkCode);
    }

    // 更新鼠标按钮状态
    leftButtonLastState = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    rightButtonLastState = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
}

// ============== 私有方法 ==============
bool InputManager::GetKeyState(int vkCode)
{
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}

bool InputManager::GetKeyLastState(int vkCode)
{
    auto it = keyLastState.find(vkCode);
    if (it != keyLastState.end())
    {
        return it->second;
    }
    return false;
}

void InputManager::SetKeyLastState(int vkCode, bool state)
{
    keyLastState[vkCode] = state;
}
