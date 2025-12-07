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

    setbkcolor(RGB(249, 247, 247)); // #F9F7F7 浅灰白背景
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
    COLORREF headColor = snake.GetId() == 0 ? RGB(63, 114, 175) : RGB(224, 133, 133); // P1:#3F72AF, P2:#E08585 莫兰迪粉
    DrawBlockWithShadow(body[0].x, body[0].y, headColor, true);

    // 在蛇头上方显示玩家标记
    wchar_t playerLabel[10];
    swprintf_s(playerLabel, L"P%d", snake.GetId() + 1);
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(RGB(17, 45, 78)); // #112D4E 深蓝色文字
    setbkmode(TRANSPARENT);
    int pixelX = GridToPixelX(body[0].x);
    int pixelY = GridToPixelY(body[0].y);
    int textWidth = textwidth(playerLabel);
    outtextxy(pixelX + (BLOCK_SIZE - textWidth) / 2, pixelY - 28, playerLabel);

    // 绘制蛇身
    COLORREF bodyColor = snake.GetId() == 0 ? RGB(63, 114, 175) : RGB(224, 133, 133); // P1:#3F72AF, P2:#E08585 莫兰迪粉
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
    // 一体化边框：绘制4个长条矩形（上、下、左、右）
    COLORREF wallColor = RGB(219, 226, 239); // #DBE2EF 灰蓝色，不抢风头
    setfillcolor(wallColor);
    setlinecolor(wallColor);

    int borderThickness = BLOCK_SIZE; // 边框厚度
    int mapWidth = MAP_WIDTH * BLOCK_SIZE;
    int mapHeight = MAP_HEIGHT * BLOCK_SIZE;

    // 上边框
    solidrectangle(0, 0, mapWidth, borderThickness);

    // 下边框
    solidrectangle(0, mapHeight - borderThickness, mapWidth, mapHeight);

    // 左边框
    solidrectangle(0, 0, borderThickness, mapHeight);

    // 右边框
    solidrectangle(mapWidth - borderThickness, 0, mapWidth, mapHeight);
}

void Renderer::DrawFood(const Food &food)
{
    COLORREF color = GetFoodColor(food.type);
    int pixelX = GridToPixelX(food.position.x);
    int pixelY = GridToPixelY(food.position.y);
    int centerX = pixelX + BLOCK_SIZE / 2;
    int centerY = pixelY + BLOCK_SIZE / 2;
    int radius = BLOCK_SIZE / 2 - 2;

    // 绘制圆形食物
    setfillcolor(color);
    setlinecolor(color);
    fillellipse(centerX - radius, centerY - radius, centerX + radius, centerY + radius);

    // 为特殊食物添加内部小圆
    if (food.type != NORMAL_FOOD && food.type != BONUS_FOOD)
    {
        int innerRadius = radius / 2;
        setfillcolor(RGB(249, 247, 247)); // #F9F7F7
        fillellipse(centerX - innerRadius, centerY - innerRadius,
                    centerX + innerRadius, centerY + innerRadius);
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
    // 绘制分割线（游戏区域和侧边栏之间）
    int dividerX = MAP_WIDTH * BLOCK_SIZE;
    setlinecolor(RGB(219, 226, 239)); // #DBE2EF 灰蓝色分割线
    setlinestyle(PS_SOLID, 3);        // 3px宽度
    line(dividerX, 0, dividerX, windowHeight);

    // 侧边栏左对齐基准位置
    int uiX = dividerX + 50; // 距离分割线50px
    int startY = 180;
    int lineHeight = 80; // 增加行间距

    // 绘制玩家颜色标记（顶部）
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(RGB(63, 114, 175)); // #3F72AF 标题颜色
    setbkmode(TRANSPARENT);
    outtextxy(uiX, 40, L"玩家标记");

    // P1圆形标记（改为圆形，和蛇一样）
    int markerY1 = 80;
    setfillcolor(RGB(63, 114, 175)); // #3F72AF
    setlinecolor(RGB(63, 114, 175));
    solidcircle(uiX + 15, markerY1 + 15, 15); // 圆形标记
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(RGB(17, 45, 78)); // #112D4E
    outtextxy(uiX + 40, markerY1 + 5, L"P1 (你)");

    // P2圆形标记
    int markerY2 = 120;
    setfillcolor(RGB(224, 133, 133)); // #E08585 莫兰迪粉
    setlinecolor(RGB(224, 133, 133));
    solidcircle(uiX + 15, markerY2 + 15, 15);
    outtextxy(uiX + 40, markerY2 + 5, L"P2");

    // 绘制统计信息（左对齐，标签+数值分层）
    wchar_t buffer[100];
    LOGFONT labelFont, valueFont;

    // 标签字体（小号，浅色）
    gettextstyle(&labelFont);
    labelFont.lfHeight = 24;
    labelFont.lfWeight = FW_NORMAL;
    wcscpy_s(labelFont.lfFaceName, L"微软雅黑");
    labelFont.lfQuality = ANTIALIASED_QUALITY;

    // 数值字体（大号，深色，加粗）
    gettextstyle(&valueFont);
    valueFont.lfHeight = 40;
    valueFont.lfWeight = FW_BOLD;
    wcscpy_s(valueFont.lfFaceName, L"微软雅黑");
    valueFont.lfQuality = ANTIALIASED_QUALITY;

    int currentY = startY;

    // 得分
    settextstyle(&labelFont);
    settextcolor(RGB(63, 114, 175)); // #3F72AF 标签颜色
    outtextxy(uiX, currentY, L"得分");
    settextstyle(&valueFont);
    settextcolor(RGB(17, 45, 78)); // #112D4E 数值颜色
    swprintf_s(buffer, L"%d", score);
    outtextxy(uiX, currentY + 28, buffer);
    currentY += lineHeight;

    // 最高分
    settextstyle(&labelFont);
    settextcolor(RGB(63, 114, 175));
    outtextxy(uiX, currentY, L"最高分");
    settextstyle(&valueFont);
    settextcolor(RGB(17, 45, 78));
    swprintf_s(buffer, L"%d", highScore);
    outtextxy(uiX, currentY + 28, buffer);
    currentY += lineHeight;

    // 长度
    settextstyle(&labelFont);
    settextcolor(RGB(63, 114, 175));
    outtextxy(uiX, currentY, L"长度");
    settextstyle(&valueFont);
    settextcolor(RGB(17, 45, 78));
    swprintf_s(buffer, L"%d", length);
    outtextxy(uiX, currentY + 28, buffer);
    currentY += lineHeight;

    // 生命
    settextstyle(&labelFont);
    settextcolor(RGB(63, 114, 175));
    outtextxy(uiX, currentY, L"生命");
    settextstyle(&valueFont);
    settextcolor(RGB(17, 45, 78));
    swprintf_s(buffer, L"%d", lives);
    outtextxy(uiX, currentY + 28, buffer);
    currentY += lineHeight;

    // 时间
    settextstyle(&labelFont);
    settextcolor(RGB(63, 114, 175));
    outtextxy(uiX, currentY, L"时间");
    settextstyle(&valueFont);
    settextcolor(RGB(17, 45, 78));
    int minutes = time / 60;
    int seconds = time % 60;
    swprintf_s(buffer, L"%02d:%02d", minutes, seconds);
    outtextxy(uiX, currentY + 28, buffer);

    // 绘制退出按钮（缩小，上移，留出底部边距）
    int sidebarWidth = windowWidth - dividerX;
    int buttonWidth = (int)(sidebarWidth * 0.7); // 侧边栏宽度的70%
    int buttonHeight = 70;
    int buttonX = dividerX + (sidebarWidth - buttonWidth) / 2; // 居中
    int buttonY = windowHeight - 150;                          // 留出底部边距

    // 使用空心描边样式（浅色背景）
    COLORREF btnBgColor = RGB(249, 247, 247);     // #F9F7F7 浅色背景
    COLORREF btnBorderColor = RGB(219, 226, 239); // #DBE2EF 边框

    setfillcolor(btnBgColor);
    setlinecolor(btnBorderColor);
    setlinestyle(PS_SOLID, 2);
    int radius = buttonHeight / 2;

    // 绘制空心胶囊按钮
    setfillcolor(btnBgColor);
    solidcircle(buttonX + radius, buttonY + radius, radius);
    solidcircle(buttonX + buttonWidth - radius, buttonY + radius, radius);
    solidrectangle(buttonX + radius, buttonY, buttonX + buttonWidth - radius, buttonY + buttonHeight);

    // 绘制边框
    setlinecolor(btnBorderColor);
    circle(buttonX + radius, buttonY + radius, radius);
    circle(buttonX + buttonWidth - radius, buttonY + radius, radius);
    line(buttonX + radius, buttonY, buttonX + buttonWidth - radius, buttonY);
    line(buttonX + radius, buttonY + buttonHeight, buttonX + buttonWidth - radius, buttonY + buttonHeight);

    // 按钮文字
    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = 28;
    f.lfWeight = FW_BOLD;
    wcscpy_s(f.lfFaceName, L"微软雅黑");
    f.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&f);
    settextcolor(RGB(63, 114, 175)); // #3F72AF 蓝色文字
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
    setfillcolor(RGB(219, 226, 239)); // #DBE2EF 灰蓝色背景
    setfillstyle(BS_SOLID);
    solidrectangle(0, 0, windowWidth, windowHeight);

    DrawTextCentered(L"游戏暂停", windowHeight / 2 - 60, 96, RGB(17, 45, 78));     // #112D4E
    DrawTextCentered(L"按空格键继续", windowHeight / 2 + 60, 48, RGB(17, 45, 78)); // #112D4E
}

void Renderer::DrawGameOverScreen(int finalScore, bool isHighScore)
{
    // 半透明背景
    setfillcolor(RGB(219, 226, 239)); // #DBE2EF
    solidrectangle(0, 0, windowWidth, windowHeight);

    // 游戏结束标题
    DrawTextCentered(L"游戏结束", windowHeight / 2 - 240, 96, RGB(17, 45, 78)); // #112D4E

    // 最终得分
    wchar_t scoreText[100];
    swprintf_s(scoreText, L"最终得分: %d", finalScore);
    DrawTextCentered(scoreText, windowHeight / 2 - 100, 64, RGB(17, 45, 78)); // #112D4E

    // 新纪录提示
    if (isHighScore)
    {
        DrawTextCentered(L"★ 新纪录！★", windowHeight / 2, 56, RGB(249, 168, 37)); // #F9A825 金黄色
    }

    // 绘制返回按钮
    int buttonX = (windowWidth - 400) / 2; // 居中
    int buttonY = windowHeight / 2 + 150;  // 调整位置
    int buttonWidth = 400;                 // 放大到2倍
    int buttonHeight = 100;                // 放大到2倍

    // 使用拼图法绘制胶囊形按钮（两个圆+一个矩形）
    COLORREF btnColor = RGB(63, 114, 175); // #3F72AF
    setfillcolor(btnColor);
    setlinecolor(btnColor);
    int radiusBtn1 = buttonHeight / 2;
    solidcircle(buttonX + radiusBtn1, buttonY + radiusBtn1, radiusBtn1);
    solidcircle(buttonX + buttonWidth - radiusBtn1, buttonY + radiusBtn1, radiusBtn1);
    solidrectangle(buttonX + radiusBtn1, buttonY, buttonX + buttonWidth - radiusBtn1, buttonY + buttonHeight);

    // 按钮文字
    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = 48;
    f.lfWeight = FW_BOLD;
    wcscpy_s(f.lfFaceName, L"微软雅黑");
    f.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&f);
    settextcolor(RGB(249, 247, 247)); // #F9F7F7 白字
    setbkmode(TRANSPARENT);
    const wchar_t *btnText = L"返回菜单";
    int textWidth = textwidth(btnText);
    int textHeight = textheight(btnText);
    int textX = buttonX + (buttonWidth - textWidth) / 2;
    int textY = buttonY + (buttonHeight - textHeight) / 2;
    outtextxy(textX, textY, btnText);

    // 提示信息
    DrawTextCentered(L"点击按钮或按回车/ESC返回", windowHeight / 2 + 300, 36, RGB(17, 45, 78)); // #112D4E
}

void Renderer::DrawMultiplayerGameOverScreen(bool playerWon, int p1Score, int p2Score, int p1Time, int p2Time)
{
    // 半透明背景
    setfillcolor(RGB(219, 226, 239)); // #DBE2EF
    solidrectangle(0, 0, windowWidth, windowHeight);

    // 游戏结束标题
    if (playerWon)
    {
        DrawTextCentered(L"★ 胜利！★", windowHeight / 2 - 280, 120, RGB(17, 45, 78)); // #112D4E
        DrawTextCentered(L"恭喜你赢得了对战！", windowHeight / 2 - 160, 64, RGB(17, 45, 78));
    }
    else
    {
        DrawTextCentered(L"战败", windowHeight / 2 - 280, 120, RGB(17, 45, 78)); // #112D4E
        DrawTextCentered(L"再接再厉,下次加油！", windowHeight / 2 - 160, 64, RGB(17, 45, 78));
    }

    // 双人得分和时长显示
    wchar_t scoreText[200];
    int centerY = windowHeight / 2 - 40;

    // P1得分（带颜色标记）
    settextstyle(48, 0, L"微软雅黑");
    settextcolor(RGB(63, 114, 175)); // #3F72AF P1颜色
    setbkmode(TRANSPARENT);
    swprintf_s(scoreText, L"P1 得分: %d", p1Score);
    int p1TextWidth = textwidth(scoreText);
    outtextxy((windowWidth - p1TextWidth) / 2, centerY, scoreText);

    // P1时长
    int p1Minutes = p1Time / 60;
    int p1Seconds = p1Time % 60;
    swprintf_s(scoreText, L"时长: %02d:%02d", p1Minutes, p1Seconds);
    settextcolor(RGB(17, 45, 78)); // #112D4E
    settextstyle(36, 0, L"微软雅黑");
    int p1TimeWidth = textwidth(scoreText);
    outtextxy((windowWidth - p1TimeWidth) / 2, centerY + 60, scoreText);

    // P2得分（带颜色标记）
    settextstyle(48, 0, L"微软雅黑");
    settextcolor(RGB(224, 133, 133)); // #E08585 P2莫兰迪粉
    swprintf_s(scoreText, L"P2 得分: %d", p2Score);
    int p2TextWidth = textwidth(scoreText);
    outtextxy((windowWidth - p2TextWidth) / 2, centerY + 120, scoreText);

    // P2时长
    int p2Minutes = p2Time / 60;
    int p2Seconds = p2Time % 60;
    swprintf_s(scoreText, L"时长: %02d:%02d", p2Minutes, p2Seconds);
    settextcolor(RGB(17, 45, 78)); // #112D4E
    settextstyle(36, 0, L"微软雅黑");
    int p2TimeWidth = textwidth(scoreText);
    outtextxy((windowWidth - p2TimeWidth) / 2, centerY + 180, scoreText);

    // 绘制返回按钮
    int buttonX = (windowWidth - 400) / 2; // 居中
    int buttonY = windowHeight / 2 + 150;  // 调整位置
    int buttonWidth = 400;                 // 放大到2倍
    int buttonHeight = 100;                // 放大到2倍

    // 使用拼图法绘制胶囊形按钮（两个圆+一个矩形）
    COLORREF btnColor = RGB(63, 114, 175); // #3F72AF
    setfillcolor(btnColor);
    setlinecolor(btnColor);
    int radiusBtn2 = buttonHeight / 2;
    solidcircle(buttonX + radiusBtn2, buttonY + radiusBtn2, radiusBtn2);
    solidcircle(buttonX + buttonWidth - radiusBtn2, buttonY + radiusBtn2, radiusBtn2);
    solidrectangle(buttonX + radiusBtn2, buttonY, buttonX + buttonWidth - radiusBtn2, buttonY + buttonHeight);

    // 按钮文字
    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = 48;
    f.lfWeight = FW_BOLD;
    wcscpy_s(f.lfFaceName, L"微软雅黑");
    f.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&f);
    settextcolor(RGB(249, 247, 247)); // #F9F7F7 白字
    setbkmode(TRANSPARENT);
    const wchar_t *btnText = L"返回菜单";
    int textWidth = textwidth(btnText);
    int textHeight = textheight(btnText);
    int textX = buttonX + (buttonWidth - textWidth) / 2;
    int textY = buttonY + (buttonHeight - textHeight) / 2;
    outtextxy(textX, textY, btnText);

    // 提示信息
    DrawTextCentered(L"点击按钮或按回车/ESC返回", windowHeight / 2 + 300, 36, RGB(17, 45, 78)); // #112D4E
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
        return RGB(249, 168, 37); // #F9A825 金黄色
    case BONUS_FOOD:
        return RGB(249, 168, 37); // #F9A825
    case MAGIC_FRUIT:
        return RGB(63, 114, 175); // #3F72AF 亮蓝
    case POISON_FRUIT:
        return RGB(219, 226, 239); // #DBE2EF 灰蓝
    case SPEED_UP:
        return RGB(63, 114, 175); // #3F72AF
    case SPEED_DOWN:
        return RGB(219, 226, 239); // #DBE2EF
    default:
        return RGB(249, 168, 37);
    }
}

COLORREF Renderer::GetWallColor(WallType type) const
{
    switch (type)
    {
    case HARD_WALL:
        return RGB(17, 45, 78); // #112D4E 深蓝色墙壁
    case SOFT_WALL:
        return RGB(17, 45, 78); // #112D4E
    case BOUNDARY:
        return RGB(17, 45, 78); // #112D4E
    default:
        return RGB(17, 45, 78);
    }
}

void Renderer::DrawBlock(int gridX, int gridY, COLORREF color, bool filled)
{
    int pixelX = GridToPixelX(gridX);
    int pixelY = GridToPixelY(gridY);

    if (filled)
    {
        setfillcolor(color);
        setlinestyle(PS_SOLID, 0);
        fillroundrect(pixelX, pixelY, pixelX + BLOCK_SIZE - 1, pixelY + BLOCK_SIZE - 1, 8, 8); // 8px圆角
    }
    else
    {
        setlinecolor(color);
        setlinestyle(PS_SOLID, 2);
        roundrect(pixelX, pixelY, pixelX + BLOCK_SIZE - 1, pixelY + BLOCK_SIZE - 1, 8, 8);
    }
}

void Renderer::DrawCapsuleButton(int x, int y, int w, int h, COLORREF color)
{
    setfillcolor(color);
    setlinecolor(color); // 边框与填充色一致，消除黑边

    int radius = h / 2; // 半圆半径 = 高度的一半

    // 1. 左边的半圆
    solidcircle(x + radius, y + radius, radius);

    // 2. 右边的半圆
    solidcircle(x + w - radius, y + radius, radius);

    // 3. 中间的矩形补齐
    solidrectangle(x + radius, y, x + w - radius, y + h);
}

void Renderer::DrawBlockWithShadow(int gridX, int gridY, COLORREF color, bool filled)
{
    int pixelX = GridToPixelX(gridX);
    int pixelY = GridToPixelY(gridY);

    if (filled)
    {
        // 先绘制阴影 (偏移2px, 颜色#DBE2EF)
        setfillcolor(RGB(219, 226, 239)); // #DBE2EF
        setlinestyle(PS_SOLID, 0);
        fillroundrect(pixelX + 2, pixelY + 2, pixelX + BLOCK_SIZE + 1, pixelY + BLOCK_SIZE + 1, 8, 8);

        // 再绘制主体
        setfillcolor(color);
        fillroundrect(pixelX, pixelY, pixelX + BLOCK_SIZE - 1, pixelY + BLOCK_SIZE - 1, 8, 8); // 8px圆角
    }
    else
    {
        setlinecolor(color);
        setlinestyle(PS_SOLID, 2);
        rectangle(pixelX, pixelY, pixelX + BLOCK_SIZE - 1, pixelY + BLOCK_SIZE - 1);
    }
}

void Renderer::GetExitButtonBounds(int &x, int &y, int &width, int &height) const
{
    int uiX = MAP_WIDTH * BLOCK_SIZE + 36;
    x = uiX;
    y = 1080 - 120; // 使用1080高度计算
    width = 270;    // 放大1.8倍
    height = 90;    // 放大1.8倍
}

// ============== 新增工具方法实现 ==============

void Renderer::DrawTextAligned(const wchar_t *text, int x, int y, int fontSize,
                               COLORREF color, int align, int width)
{
    settextstyle(fontSize, 0, L"微软雅黑");
    settextcolor(color);
    setbkmode(TRANSPARENT);

    int textWidth = textwidth(text);
    int finalX = x;

    // 计算对齐位置 (align: 0=LEFT, 1=CENTER, 2=RIGHT)
    if (align == 1) // CENTER
    {
        finalX = x + (width > 0 ? (width - textWidth) / 2 : -textWidth / 2);
    }
    else if (align == 2) // RIGHT
    {
        finalX = x - textWidth;
    }

    outtextxy(finalX, y, text);
}

void Renderer::DrawRoundRect(int x, int y, int width, int height, int radius,
                             COLORREF fillColor, COLORREF borderColor,
                             bool hasShadow, int shadowOffset)
{
    // 绘制阴影
    if (hasShadow)
    {
        COLORREF shadowColor = RGB(17, 45, 78); // #112D4E
        setfillcolor(shadowColor);
        setlinecolor(shadowColor);
        solidroundrect(x + shadowOffset, y + shadowOffset,
                       x + width + shadowOffset, y + height + shadowOffset,
                       radius, radius);
    }

    // 绘制主体
    setfillcolor(fillColor);
    setlinecolor(borderColor);
    solidroundrect(x, y, x + width, y + height, radius, radius);

    // 绘制边框
    if (borderColor != fillColor)
    {
        setlinecolor(borderColor);
        roundrect(x, y, x + width, y + height, radius, radius);
    }
}

bool Renderer::IsMouseInRect(int mouseX, int mouseY, int x, int y, int width, int height) const
{
    return mouseX >= x && mouseX <= x + width &&
           mouseY >= y && mouseY <= y + height;
}
