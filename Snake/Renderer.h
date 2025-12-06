#pragma once
#include "Common.h"
#include "Snake.h"
#include "GameMap.h"
#include "FoodManager.h"
#include <graphics.h>
#include <string>

// ============== 渲染器类 ==============
// 封装所有EasyX绘图操作
class Renderer
{
private:
    int windowWidth;
    int windowHeight;
    bool initialized;
    bool ownsWindow; // 是否拥有窗口所有权

public:
    // ============== 构造与析构 ==============
    Renderer();
    ~Renderer();

    // ============== 初始化与清理 ==============
    /**
     * @brief 初始化渲染器
     * @param width 窗口宽度
     * @param height 窗口高度
     * @param title 窗口标题
     * @param createWindow 是否创建窗口（默认false，由外部管理）
     */
    bool Init(int width, int height, const wchar_t *title = L"贪吃蛇游戏", bool createWindow = false);

    /**
     * @brief 关闭图形窗口
     */
    void Close();

    /**
     * @brief 清空屏幕
     */
    void Clear(COLORREF bgColor = BLACK);

    // ============== 游戏元素绘制 ==============
    /**
     * @brief 绘制蛇
     */
    void DrawSnake(const Snake &snake);

    /**
     * @brief 绘制多条蛇
     */
    void DrawSnakes(const std::vector<Snake *> &snakes);

    /**
     * @brief 绘制地图（墙壁）
     */
    void DrawMap(const GameMap &map);

    /**
     * @brief 绘制食物
     */
    void DrawFood(const Food &food);

    /**
     * @brief 绘制所有食物
     */
    void DrawFoods(const FoodManager &foodMgr);

    // ============== UI绘制 ==============
    /**
     * @brief 绘制游戏UI信息（包含退出按钮）
     * @param score 当前得分
     * @param highScore 历史最高分
     * @param length 蛇长度
     * @param lives 剩余生命
     * @param time 游戏时间（秒）
     */
    void DrawUI(int score, int highScore, int length, int lives, int time);

    /**
     * @brief 获取退出按钮区域（用于点击检测）
     * @return 返回按钮的(x, y, width, height)
     */
    void GetExitButtonBounds(int &x, int &y, int &width, int &height) const;

    /**
     * @brief 绘制暂停界面
     */
    void DrawPauseScreen();

    /**
     * @brief 绘制游戏结束界面
     */
    void DrawGameOverScreen(int finalScore, bool isHighScore);
    void DrawMultiplayerGameOverScreen(bool playerWon, int finalScore);

    // ============== 工具方法 ==============
    /**
     * @brief 开始批量绘图
     */
    void BeginBatch();

    /**
     * @brief 结束批量绘图并刷新屏幕
     */
    void EndBatch();

    /**
     * @brief 绘制文本（居中）
     */
    void DrawTextCentered(const wchar_t *text, int y, int fontSize, COLORREF color = WHITE);

    /**
     * @brief 绘制矩形框
     */
    void DrawRect(int x, int y, int width, int height, COLORREF color, bool filled = false);

private:
    /**
     * @brief 将格子坐标转换为像素坐标
     */
    int GridToPixelX(int gridX) const;
    int GridToPixelY(int gridY) const;

    /**
     * @brief 根据食物类型获取颜色
     */
    COLORREF GetFoodColor(FoodType type) const;

    /**
     * @brief 根据墙壁类型获取颜色
     */
    COLORREF GetWallColor(WallType type) const;

    /**
     * @brief 绘制单个格子
     */
    void DrawBlock(int gridX, int gridY, COLORREF color, bool filled = true);
};
