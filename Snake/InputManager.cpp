#include "InputManager.h"
#include <vector>

// ============== 构造与析构 ==============
InputManager::InputManager()
    : leftButtonLastState(false), rightButtonLastState(false),
      leftButtonJustPressed(false), rightButtonJustPressed(false),
      lastMouseX(0), lastMouseY(0)
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
    // 返回并清除标志（一次性消费）
    bool result = leftButtonJustPressed;
    leftButtonJustPressed = false;
    return result;
}

bool InputManager::IsRightButtonJustPressed()
{
    // 返回并清除标志（一次性消费）
    bool result = rightButtonJustPressed;
    rightButtonJustPressed = false;
    return result;
}

void InputManager::ClearMouseQueue()
{
    while (MouseHit())
    {
        GetMouseMsg();
    }
}

bool InputManager::GetNextMouseMessage(MOUSEMSG &msg)
{
    if (!mouseMessageBuffer.empty())
    {
        msg = mouseMessageBuffer.front();
        mouseMessageBuffer.erase(mouseMessageBuffer.begin());
        return true;
    }
    return false;
}

bool InputManager::IsMouseClickInRect(int x, int y, int width, int height, UINT clickType)
{
    // 遍历缓冲区中的所有消息
    for (auto it = mouseMessageBuffer.begin(); it != mouseMessageBuffer.end(); ++it)
    {
        if (it->uMsg == clickType)
        {
            // 检测点击位置是否在矩形内
            if (it->x >= x && it->x <= x + width &&
                it->y >= y && it->y <= y + height)
            {
                // 找到匹配的点击，从缓冲区移除并返回true
                mouseMessageBuffer.erase(it);
                return true;
            }
        }
    }
    return false;
}

void InputManager::GetMousePosition(int &x, int &y)
{
    x = lastMouseX;
    y = lastMouseY;
}

void InputManager::ClearMouseBuffer()
{
    mouseMessageBuffer.clear();
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

    // 读取所有鼠标消息到缓冲区（不丢失任何消息）
    // 使用 MouseHit() 是非阻塞的，不会卡住程序
    while (MouseHit())
    {
        MOUSEMSG msg = GetMouseMsg();
        mouseMessageBuffer.push_back(msg);

        // 更新鼠标位置缓存
        lastMouseX = msg.x;
        lastMouseY = msg.y;
    }

    // 检测鼠标按钮边沿（从未按下⇒按下）
    bool newLeftState = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    bool newRightState = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

    // 边沿检测：当前按下 && 上一帧未按下
    if (newLeftState && !leftButtonLastState)
    {
        leftButtonJustPressed = true;
        OutputDebugStringA("[InputManager::Update] Left button JUST PRESSED detected!\n");
    }

    if (newRightState && !rightButtonLastState)
    {
        rightButtonJustPressed = true;
    }

    // 更新状态
    leftButtonLastState = newLeftState;
    rightButtonLastState = newRightState;
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
