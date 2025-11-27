#pragma once
#include "Common.h"

// 前向声明
class Snake;
class GameMap;

// ============== 控制器接口 ==============
// 这是解耦的核心：所有控制器都实现这个接口
// 无论是键盘控制、网络控制还是AI控制，都通过这个接口与蛇交互

class IController
{
public:
    virtual ~IController() {}

    // ============== 核心接口方法 ==============

    /**
     * @brief 决策函数 - 根据当前蛇和地图状态，决定下一步的移动方向
     * @param snake 当前蛇的引用（只读）
     * @param map 当前地图的引用（只读）
     * @return 决策的移动方向
     */
    virtual Direction MakeDecision(const Snake &snake, const GameMap &map) = 0;

    /**
     * @brief 更新控制器状态（可选实现）
     * @details 某些控制器可能需要每帧更新（如网络控制器接收数据包）
     */
    virtual void Update() {}

    /**
     * @brief 初始化控制器（可选实现）
     */
    virtual void Init() {}

    /**
     * @brief 获取控制器类型名称（用于调试）
     */
    virtual const char *GetTypeName() const { return "IController"; }
};
