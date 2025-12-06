#include "Renderer.h"
#include <sstream>

// ============== 构造与析构 ==============
Renderer::Renderer() : windowWidth(0), windowHeight(0), initialized(false), ownsWindow(false)
{
}

Renderer::~Renderer()
{
    Close();
}

// ============== 初始化与清理 ==============
bool Renderer::Init(int width, int height, const wchar_t *title, bool createWindow)
{
    ownsWindow = createWindow;

    if (createWindow)
    {
        initgraph(width, height);
    }

    // 设置窗口属性
    HWND hwnd = GetHWnd();
    if (hwnd)
    {
        SetWindowText(hwnd, title);
        SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
    }

    setbkcolor(BLACK);
    cleardevice();

    // 启用文字抗锯齿（ClearType），提升文字清晰度
    LOGFONT f;
    gettextstyle(&f);
    f.lfQuality = CLEARTYPE_QUALITY;
    wcscpy_s(f.lfFaceName, L"微软雅黑");
    settextstyle(&f);

    windowWidth = width;
    windowHeight = height;
    initialized = true;
    return true;
}

void Renderer::Close()
{
    if (initialized && ownsWindow)
    {
        closegraph();
    }
    initialized = false;
}

void Renderer::Clear(COLORREF bgColor)
{
    setbkcolor(bgColor);
    cleardevice();
}

// ============== 游戏元素绘制 ==============
void Renderer::DrawSnake(const Snake &snake)
{
    const auto &body = snake.GetBody();
    if (body.empty())
        return;

    // 绘制蛇头（不同颜色）
    COLORREF headColor = snake.GetId() == 0 ? RGB(0, 255, 0) : RGB(0, 200, 255);
    DrawBlock(body[0].x, body[0].y, headColor, true);

    // 绘制蛇身
    COLORREF bodyColor = snake.GetId() == 0 ? RGB(0, 200, 0) : RGB(0, 150, 200);
    for (size_t i = 1; i < body.size(); ++i)
    {
        DrawBlock(body[i].x, body[i].y, bodyColor, true);
    }
}

void Renderer::DrawSnakes(const std::vector<Snake *> &snakes)
{
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
    // 遍历地图，绘制所有墙壁
    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        for (int x = 0; x < MAP_WIDTH; ++x)
        {
            WallType wallType = map.GetWallType(Point(x, y));
            if (wallType != NO_WALL)
            {
                COLORREF color = GetWallColor(wallType);
                DrawBlock(x, y, color, true);
            }
        }
    }
}

void Renderer::DrawFood(const Food &food)
{
    COLORREF color = GetFoodColor(food.type);
    DrawBlock(food.position.x, food.position.y, color, true);

    // 为特殊食物添加标记（绘制内部小方块）
    if (food.type != NORMAL_FOOD && food.type != BONUS_FOOD)
    {
        int pixelX = GridToPixelX(food.position.x);
        int pixelY = GridToPixelY(food.position.y);
        int margin = BLOCK_SIZE / 4;

        setfillcolor(WHITE);
        solidrectangle(
            pixelX + margin,
            pixelY + margin,
            pixelX + BLOCK_SIZE - margin - 1,
            pixelY + BLOCK_SIZE - margin - 1);
    }
}

void Renderer::DrawFoods(const FoodManager &foodMgr)
{
    const auto &foods = foodMgr.GetAllFoods();
    for (const auto &food : foods)
    {
        DrawFood(food);
    }
}

// ============== UI绘制 ==============
void Renderer::DrawUI(int score, int highScore, int length, int lives, int time)
{
    // 设置文本样式
    settextstyle(20, 0, L"微软雅黑");
    settextcolor(WHITE);

    // 在游戏区域右侧绘制UI信息
    int uiX = MAP_WIDTH * BLOCK_SIZE + 20;
    int startY = 50;
    int lineHeight = 40;

    wchar_t buffer[100];

    // 得分
    swprintf_s(buffer, L"得分: %d", score);
    outtextxy(uiX, startY, buffer);

    // 历史最高分
    swprintf_s(buffer, L"最高分: %d", highScore);
    outtextxy(uiX, startY + lineHeight, buffer);

    // 蛇长度
    swprintf_s(buffer, L"长度: %d", length);
    outtextxy(uiX, startY + lineHeight * 2, buffer);

    // 剩余生命
    swprintf_s(buffer, L"生命: %d", lives);
    outtextxy(uiX, startY + lineHeight * 3, buffer);

    // 游戏时间
    int minutes = time / 60;
    int seconds = time % 60;
    swprintf_s(buffer, L"时间: %02d:%02d", minutes, seconds);
    outtextxy(uiX, startY + lineHeight * 4, buffer);

    // 绘制分隔线
    setlinecolor(RGB(100, 100, 100));
    line(MAP_WIDTH * BLOCK_SIZE + 10, 0, MAP_WIDTH * BLOCK_SIZE + 10, windowHeight);

    // 绘制退出按钮
    int buttonX = uiX;
    int buttonY = windowHeight - 80;
    int buttonWidth = 150;
    int buttonHeight = 50;

    setfillcolor(RGB(200, 50, 50));
    setlinecolor(RGB(255, 100, 100));
    setlinestyle(PS_SOLID, 2);
    fillrectangle(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);

    // 按钮文字
    settextstyle(20, 0, L"微软雅黑");
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    const wchar_t *btnText = L"退出游戏";
    int textWidth = textwidth(btnText);
    int textHeight = textheight(btnText);
    int textX = buttonX + (buttonWidth - textWidth) / 2;
    int textY = buttonY + (buttonHeight - textHeight) / 2;
    outtextxy(textX, textY, btnText);
}

void Renderer::DrawPauseScreen()
{
    // 半透明背景
    setfillcolor(RGB(0, 0, 0));
    setfillstyle(BS_SOLID);
    solidrectangle(0, 0, windowWidth, windowHeight);

    DrawTextCentered(L"游戏暂停", windowHeight / 2 - 30, 48, YELLOW);
    DrawTextCentered(L"按空格键继续", windowHeight / 2 + 30, 24, WHITE);
}

void Renderer::DrawGameOverScreen(int finalScore, bool isHighScore)
{
    // 半透明背景
    setfillcolor(RGB(50, 0, 0));
    solidrectangle(0, 0, windowWidth, windowHeight);

    // 游戏结束标题
    DrawTextCentered(L"游戏结束", windowHeight / 2 - 120, 48, RED);

    // 最终得分
    wchar_t scoreText[100];
    swprintf_s(scoreText, L"最终得分: %d", finalScore);
    DrawTextCentered(scoreText, windowHeight / 2 - 50, 32, WHITE);

    // 新纪录提示
    if (isHighScore)
    {
        DrawTextCentered(L"★ 新纪录！★", windowHeight / 2, 28, YELLOW);
    }

    // 绘制返回按钮
    int buttonX = 300;
    int buttonY = 400;
    int buttonWidth = 200;
    int buttonHeight = 50;

    setfillcolor(RGB(70, 130, 180));
    setlinecolor(RGB(100, 200, 255));
    setlinestyle(PS_SOLID, 2);
    fillrectangle(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);

    // 按钮文字
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    const wchar_t *btnText = L"返回菜单";
    int textWidth = textwidth(btnText);
    int textHeight = textheight(btnText);
    int textX = buttonX + (buttonWidth - textWidth) / 2;
    int textY = buttonY + (buttonHeight - textHeight) / 2;
    outtextxy(textX, textY, btnText);

    // 提示信息
    DrawTextCentered(L"点击按钮或按回车/ESC返回", windowHeight / 2 + 150, 18, RGB(150, 150, 150));
}

void Renderer::DrawMultiplayerGameOverScreen(bool playerWon, int finalScore)
{
    // 半透明背景
    if (playerWon)
    {
        setfillcolor(RGB(0, 50, 0)); // 绿色背景（胜利）
    }
    else
    {
        setfillcolor(RGB(50, 0, 0)); // 红色背景（失败）
    }
    solidrectangle(0, 0, windowWidth, windowHeight);

    // 游戏结束标题
    if (playerWon)
    {
        DrawTextCentered(L"★ 胜利！★", windowHeight / 2 - 120, 60, YELLOW);
        DrawTextCentered(L"恭喜你赢得了对战！", windowHeight / 2 - 50, 32, GREEN);
    }
    else
    {
        DrawTextCentered(L"战败", windowHeight / 2 - 120, 60, RED);
        DrawTextCentered(L"再接再厉，下次加油！", windowHeight / 2 - 50, 32, RGB(255, 150, 150));
    }

    // 最终得分
    wchar_t scoreText[100];
    swprintf_s(scoreText, L"你的得分: %d", finalScore);
    DrawTextCentered(scoreText, windowHeight / 2 + 10, 28, WHITE);

    // 绘制返回按钮
    int buttonX = 300;
    int buttonY = 400;
    int buttonWidth = 200;
    int buttonHeight = 50;

    setfillcolor(RGB(70, 130, 180));
    setlinecolor(RGB(100, 200, 255));
    setlinestyle(PS_SOLID, 2);
    fillrectangle(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);

    // 按钮文字
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    const wchar_t *btnText = L"返回菜单";
    int textWidth = textwidth(btnText);
    int textHeight = textheight(btnText);
    int textX = buttonX + (buttonWidth - textWidth) / 2;
    int textY = buttonY + (buttonHeight - textHeight) / 2;
    outtextxy(textX, textY, btnText);

    // 提示信息
    DrawTextCentered(L"点击按钮或按回车/ESC返回", windowHeight / 2 + 150, 18, RGB(150, 150, 150));
}

// ============== 工具方法 ==============
void Renderer::BeginBatch()
{
    BeginBatchDraw();
}

void Renderer::EndBatch()
{
    EndBatchDraw();
}

void Renderer::DrawTextCentered(const wchar_t *text, int y, int fontSize, COLORREF color)
{
    settextstyle(fontSize, 0, L"微软雅黑");
    settextcolor(color);
    int textWidth = textwidth(text);
    outtextxy((windowWidth - textWidth) / 2, y, text);
}

void Renderer::DrawRect(int x, int y, int width, int height, COLORREF color, bool filled)
{
    setlinecolor(color);
    if (filled)
    {
        setfillcolor(color);
        solidrectangle(x, y, x + width, y + height);
    }
    else
    {
        rectangle(x, y, x + width, y + height);
    }
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
    int pixelX = GridToPixelX(gridX);
    int pixelY = GridToPixelY(gridY);

    if (filled)
    {
        setfillcolor(color);
        solidrectangle(pixelX, pixelY, pixelX + BLOCK_SIZE - 1, pixelY + BLOCK_SIZE - 1);

        // 绘制边框以区分不同格子
        setlinecolor(RGB(30, 30, 30));
        rectangle(pixelX, pixelY, pixelX + BLOCK_SIZE - 1, pixelY + BLOCK_SIZE - 1);
    }
    else
    {
        setlinecolor(color);
        rectangle(pixelX, pixelY, pixelX + BLOCK_SIZE - 1, pixelY + BLOCK_SIZE - 1);
    }
}

void Renderer::GetExitButtonBounds(int &x, int &y, int &width, int &height) const
{
    int uiX = MAP_WIDTH * BLOCK_SIZE + 20;
    x = uiX;
    y = windowHeight - 80;
    width = 150;
    height = 50;
}
