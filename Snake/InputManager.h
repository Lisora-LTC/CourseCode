#pragma once
#include <windows.h>
#include <graphics.h>
#include <map>

// ============== 统一输入管理器 ==============
// 负责处理UI层面的输入（菜单、暂停、游戏结束等场景）
// 不干扰游戏逻辑层的输入（KeyboardController）
class InputManager
{
private:
    // 键盘状态（用于边沿检测/防抖）
    std::map<int, bool> keyLastState;

    // 鼠标按钮状态
    bool leftButtonLastState;
    bool rightButtonLastState;

public:
    // ============== 构造与析构 ==============
    InputManager();
    ~InputManager();

    // ============== 键盘输入 ==============

    /**
     * @brief 检测按键是否刚按下（边沿触发，自动防抖）
     * @param vkCode 虚拟键码 (如 VK_UP, VK_RETURN, VK_ESCAPE)
     * @return true 如果按键刚从未按下变为按下
     */
    bool IsKeyJustPressed(int vkCode);

    /**
     * @brief 检测按键是否持续按下（电平触发）
     * @param vkCode 虚拟键码
     * @return true 如果按键当前处于按下状态
     */
    bool IsKeyHeld(int vkCode);

    // ============== 鼠标输入 ==============

    /**
     * @brief 获取最新的鼠标消息（自动清空消息队列）
     * @param msg 输出参数，存储鼠标消息
     * @return true 如果有消息可用
     */
    bool GetLatestMouseMessage(MOUSEMSG &msg);

    /**
     * @brief 检测鼠标左键是否刚点击（边沿触发）
     * @return true 如果左键刚按下
     */
    bool IsLeftButtonJustPressed();

    /**
     * @brief 检测鼠标右键是否刚点击（边沿触发）
     * @return true 如果右键刚按下
     */
    bool IsRightButtonJustPressed();

    /**
     * @brief 清空鼠标消息队列（防止输入积压）
     */
    void ClearMouseQueue();

    // ============== 窗口相关 ==============

    /**
     * @brief 检查 EasyX 窗口是否被关闭
     * @return true 如果窗口已关闭
     */
    bool IsWindowClosed();

    // ============== 状态更新 ==============

    /**
     * @brief 更新输入状态（每帧调用）
     * 更新按键和鼠标的上一帧状态，用于边沿检测
     */
    void Update();

private:
    /**
     * @brief 获取按键当前状态
     */
    bool GetKeyState(int vkCode);

    /**
     * @brief 获取按键上一帧状态
     */
    bool GetKeyLastState(int vkCode);

    /**
     * @brief 设置按键上一帧状态
     */
    void SetKeyLastState(int vkCode, bool state);
};
