#pragma once
#include "IController.h"
#include <windows.h>

// ============== 键盘控制器 ==============
// 用于本地玩家控制
class KeyboardController : public IController
{
private:
    Direction lastDirection; // 上一次的方向（防止180度转向）
    int playerIndex;         // 玩家编号（用于多人本地对战）

    // 键位配置（支持WASD和方向键）
    int keyUp;
    int keyDown;
    int keyLeft;
    int keyRight;

    // 输入缓冲系统
    Direction bufferedInput; // 缓存的输入
    bool hasBufferedInput;   // 是否有缓存的输入

    // 按键状态（用于边沿检测）
    bool keyState[4]; // UP, DOWN, LEFT, RIGHT 的状态

public:
    // ============== 构造与析构 ==============
    KeyboardController();
    KeyboardController(int player); // player: 0=P1(WASD), 1=P2(方向键)
    ~KeyboardController() override;

    // ============== 实现接口方法 ==============
    Direction MakeDecision(const Snake &snake, const GameMap &map) override;
    void Init() override;
    const char *GetTypeName() const override { return "KeyboardController"; }

    /**
     * @brief 高频采样输入（每帧调用，缓存按键）
     */
    void CacheInput();

    /**
     * @brief 检查是否有缓冲的输入
     */
    bool HasBufferedInput() const { return hasBufferedInput; }

    /**
     * @brief 获取缓冲的输入（不清除缓冲）
     */
    Direction GetBufferedInput() const { return bufferedInput; }

private:
    /**
     * @brief 检测键盘输入
     */
    Direction DetectInput();

    /**
     * @brief 初始化键位配置
     */
    void InitKeyBindings(int player);

    /**
     * @brief 检查按键是否按下（使用GetAsyncKeyState）
     */
    bool IsKeyPressed(int vkCode);
};
