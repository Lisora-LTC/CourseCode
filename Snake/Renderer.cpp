#include "Renderer.h"
#include <sstream>

// ============== 构造与析构 ==============
Renderer::Renderer() : windowWidth(0), windowHeight(0), initialized(false)
{
    // TODO: 初始化
}

Renderer::~Renderer()
{
    Close();
}

// ============== 初始化与清理 ==============
bool Renderer::Init(int width, int height, const wchar_t *title)
{
    // TODO: 初始化EasyX图形窗口
    // initgraph(width, height);
    // setbkcolor(BLACK);
    // cleardevice();

    windowWidth = width;
    windowHeight = height;
    initialized = true;
    return true;
}

void Renderer::Close()
{
    // TODO: 关闭图形窗口
    // closegraph();
    initialized = false;
}

void Renderer::Clear(COLORREF bgColor)
{
    // TODO: 清空屏幕
    // setbkcolor(bgColor);
    // cleardevice();
}

// ============== 游戏元素绘制 ==============
void Renderer::DrawSnake(const Snake &snake)
{
    // TODO: 绘制蛇
    // 1. 获取蛇的body
    // 2. 遍历每个点，绘制方块
    // 3. 蛇头可以用不同颜色区分
}

void Renderer::DrawSnakes(const std::vector<Snake *> &snakes)
{
    // TODO: 绘制多条蛇
    for (auto snake : snakes)
    {
        if (snake && snake->IsAlive())
        {
            DrawSnake(*snake);
        }
    }
}

void Renderer::DrawMap(const GameMap &map)
{
    // TODO: 绘制地图墙壁
    // 遍历地图，绘制所有墙壁
}

void Renderer::DrawFood(const Food &food)
{
    // TODO: 绘制单个食物
    // 根据食物类型选择不同颜色
    // COLORREF color = GetFoodColor(food.type);
    // DrawBlock(food.position.x, food.position.y, color);
}

void Renderer::DrawFoods(const FoodManager &foodMgr)
{
    // TODO: 绘制所有食物
    const auto &foods = foodMgr.GetAllFoods();
    for (const auto &food : foods)
    {
        DrawFood(food);
    }
}

// ============== UI绘制 ==============
void Renderer::DrawUI(int score, int highScore, int length, int lives, int time)
{
    // TODO: 绘制UI信息
    // 在屏幕右侧或上方显示信息
    // 示例：
    // wchar_t buffer[100];
    // swprintf_s(buffer, L"得分: %d", score);
    // outtextxy(10, 10, buffer);
}

void Renderer::DrawPauseScreen()
{
    // TODO: 绘制暂停界面
    DrawTextCentered(L"游戏暂停", windowHeight / 2, 48, YELLOW);
    DrawTextCentered(L"按任意键继续", windowHeight / 2 + 60, 24, WHITE);
}

void Renderer::DrawGameOverScreen(int finalScore, bool isHighScore)
{
    // TODO: 绘制游戏结束界面
    DrawTextCentered(L"游戏结束", windowHeight / 2 - 60, 48, RED);

    wchar_t scoreText[100];
    swprintf_s(scoreText, L"最终得分: %d", finalScore);
    DrawTextCentered(scoreText, windowHeight / 2, 32, WHITE);

    if (isHighScore)
    {
        DrawTextCentered(L"新纪录！", windowHeight / 2 + 50, 28, YELLOW);
    }
}

// ============== 工具方法 ==============
void Renderer::BeginBatch()
{
    // TODO: 开始批量绘图
    // BeginBatchDraw();
}

void Renderer::EndBatch()
{
    // TODO: 结束批量绘图
    // EndBatchDraw();
}

void Renderer::DrawTextCentered(const wchar_t *text, int y, int fontSize, COLORREF color)
{
    // TODO: 绘制居中文本
    // settextstyle(fontSize, 0, L"微软雅黑");
    // settextcolor(color);
    // int textWidth = textwidth(text);
    // outtextxy((windowWidth - textWidth) / 2, y, text);
}

void Renderer::DrawRect(int x, int y, int width, int height, COLORREF color, bool filled)
{
    // TODO: 绘制矩形
    // setlinecolor(color);
    // if (filled) {
    //     setfillcolor(color);
    //     fillrectangle(x, y, x + width, y + height);
    // } else {
    //     rectangle(x, y, x + width, y + height);
    // }
}

// ============== 私有方法 ==============
int Renderer::GridToPixelX(int gridX) const
{
    return gridX * BLOCK_SIZE;
}

int Renderer::GridToPixelY(int gridY) const
{
    return gridY * BLOCK_SIZE;
}

COLORREF Renderer::GetFoodColor(FoodType type) const
{
    // TODO: 根据食物类型返回颜色
    switch (type)
    {
    case NORMAL_FOOD:
        return RGB(255, 0, 0); // 红色
    case BONUS_FOOD:
        return RGB(255, 215, 0); // 金色
    case MAGIC_FRUIT:
        return RGB(0, 255, 255); // 青色
    case POISON_FRUIT:
        return RGB(128, 0, 128); // 紫色
    case SPEED_UP:
        return RGB(255, 165, 0); // 橙色
    case SPEED_DOWN:
        return RGB(0, 0, 255); // 蓝色
    default:
        return RGB(255, 0, 0);
    }
}

COLORREF Renderer::GetWallColor(WallType type) const
{
    // TODO: 根据墙壁类型返回颜色
    switch (type)
    {
    case HARD_WALL:
        return RGB(100, 100, 100); // 深灰
    case SOFT_WALL:
        return RGB(150, 150, 150); // 浅灰
    case BOUNDARY:
        return RGB(255, 255, 255); // 白色
    default:
        return RGB(0, 0, 0);
    }
}

void Renderer::DrawBlock(int gridX, int gridY, COLORREF color, bool filled)
{
    // TODO: 绘制单个格子
    int pixelX = GridToPixelX(gridX);
    int pixelY = GridToPixelY(gridY);

    // setfillcolor(color);
    // setlinecolor(color);
    // if (filled) {
    //     fillrectangle(pixelX, pixelY, pixelX + BLOCK_SIZE - 1, pixelY + BLOCK_SIZE - 1);
    // } else {
    //     rectangle(pixelX, pixelY, pixelX + BLOCK_SIZE - 1, pixelY + BLOCK_SIZE - 1);
    // }
}
