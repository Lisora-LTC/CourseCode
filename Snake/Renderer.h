#pragma once
#include "Common.h"
#include "Snake.h"
#include "GameMap.h"
#include "FoodManager.h"
#include "UIComponent.h"
#include <graphics.h>
#include <string>
#include <memory>

// ============== 渲染器类 ==============
// 封装所有EasyX绘图操作
class Renderer
{
private:
    int windowWidth;
    int windowHeight;
    bool initialized;
    bool ownsWindow; // 是否拥有窗口所有权

    // UI组件
    std::unique_ptr<UIButton> exitButton; // 退出按钮

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
     * @param wallCollisions 撞墙次数（高级版用）
     * @param mode 游戏模式
     */
    void DrawUI(int score, int highScore, int length, int lives, int time, int wallCollisions = 0, GameMode mode = SINGLE);

    /**
     * @brief 绘制食物图例说明（旧版，单列布局）
     */
    void DrawFoodLegend(int startY);

    /**
     * @brief 绘制食物图例说明（新版，双列布局）
     */
    void DrawFoodLegendDualColumn();

    /**
     * @brief 获取退出按钮区域（用于点击检测）
     * @return 返回按钮的(x, y, width, height)
     */
    void GetExitButtonBounds(int &x, int &y, int &width, int &height) const;

    /**
     * @brief 获取退出按钮UI组件
     */
    UIButton *GetExitButton() { return exitButton.get(); }

    /**
     * @brief 绘制暂停界面
     */
    void DrawPauseScreen();

    /**
     * @brief 绘制游戏结束界面
     */
    void DrawGameOverScreen(int finalScore, bool isHighScore);

    /**
     * @brief 绘制多人游戏结束界面
     * @param playerWon P1是否胜利
     * @param p1Score P1得分
     * @param p2Score P2得分
     * @param p1Time P1游戏时长（秒）
     * @param p2Time P2游戏时长（秒）
     */
    void DrawMultiplayerGameOverScreen(bool playerWon, int p1Score, int p2Score, int p1Time, int p2Time);

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
     * @brief 绘制文本（支持多种对齐方式）
     * @param text 文本内容
     * @param x X坐标
     * @param y Y坐标
     * @param fontSize 字体大小
     * @param color 文本颜色
     * @param align 对齐方式（LEFT/CENTER/RIGHT）
     * @param width 文本区域宽度（用于对齐计算，0表示不限制）
     */
    void DrawTextAligned(const wchar_t *text, int x, int y, int fontSize,
                         COLORREF color, int align = 0, int width = 0);

    /**
     * @brief 绘制矩形框
     */
    void DrawRect(int x, int y, int width, int height, COLORREF color, bool filled = false);

    /**
     * @brief 绘制圆角矩形（带可选阴影）
     * @param x X坐标
     * @param y Y坐标
     * @param width 宽度
     * @param height 高度
     * @param radius 圆角半径
     * @param fillColor 填充颜色
     * @param borderColor 边框颜色
     * @param hasShadow 是否绘制阴影
     * @param shadowOffset 阴影偏移量
     */
    void DrawRoundRect(int x, int y, int width, int height, int radius,
                       COLORREF fillColor, COLORREF borderColor = RGB(0, 0, 0),
                       bool hasShadow = false, int shadowOffset = 5);

    /**
     * @brief 检测鼠标是否在矩形区域内
     */
    bool IsMouseInRect(int mouseX, int mouseY, int x, int y, int width, int height) const;

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

    /**
     * @brief 绘制带阴影的格子
     */
    void DrawBlockWithShadow(int gridX, int gridY, COLORREF color, bool filled = true);

    /**
     * @brief 绘制胶囊形按钮（拼图法：两个圆+一个矩形）
     * @param x 按钮左上角X坐标
     * @param y 按钮左上角Y坐标
     * @param w 按钮宽度
     * @param h 按钮高度
     * @param color 按钮颜色
     */
    void DrawCapsuleButton(int x, int y, int w, int h, COLORREF color);
};
