#include "Renderer.h"
#include <sstream>

// ============== 构造与析构 ==============
Renderer::Renderer() : windowWidth(0), windowHeight(0), initialized(false), ownsWindow(false)
{
    // 初始化退出按钮（位置和大小在Init后设置）
    exitButton = nullptr;
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

    // 初始化退出按钮（底部Y:960，320x80胶囊按钮，主菜单同款）
    int buttonWidth = 320;
    int buttonHeight = 80;
    int buttonX = SIDEBAR_CENTER_X - buttonWidth / 2; // 侧边栏中轴线居中 (1680-160=1520)
    int buttonY = 960;

    exitButton = std::make_unique<UIButton>(buttonX, buttonY, buttonWidth, buttonHeight, L"退出游戏", BUTTON_CAPSULE);
    // 主菜单同款：实心亮蓝 #3F72AF，悬停变亮 #5584BC
    exitButton->SetColors(RGB(63, 114, 175), RGB(85, 132, 188), RGB(17, 45, 78)); // #3F72AF -> #5584BC -> #112D4E
    exitButton->SetTextColor(RGB(249, 247, 247), RGB(255, 255, 255));             // #F9F7F7 纯白
    exitButton->SetFontSize(32);                                                  // 32px粗体（微软雅黑Bold）
    exitButton->SetShadow(false, 0);                                              // 去黑边，无描边

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

    // 绘制蛇身
    COLORREF bodyColor = snake.GetId() == 0 ? RGB(63, 114, 175) : RGB(224, 133, 133); // P1:#3F72AF, P2:#E08585 莫兰迪粉
    for (size_t i = 1; i < body.size(); ++i)
    {
        DrawBlock(body[i].x, body[i].y, bodyColor, true);
    }

    // 在蛇头上方显示玩家标记（最后绘制，避免被身体遮挡）
    wchar_t playerLabel[10];
    swprintf_s(playerLabel, L"P%d", snake.GetId() + 1);
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(RGB(17, 45, 78)); // #112D4E 深蓝色文字
    setbkmode(TRANSPARENT);
    int pixelX = GridToPixelX(body[0].x);
    int pixelY = GridToPixelY(body[0].y);
    int textWidth = textwidth(playerLabel);
    outtextxy(pixelX + (BLOCK_SIZE - textWidth) / 2, pixelY - 28, playerLabel);
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

    int borderThickness = BLOCK_SIZE;        // 边框厚度 36px
    int mapWidth = MAP_WIDTH * BLOCK_SIZE;   // 1368px
    int mapHeight = MAP_HEIGHT * BLOCK_SIZE; // 1008px

    // 上边框 (加上GAME_AREA_X/Y偏移)
    solidrectangle(GAME_AREA_X, GAME_AREA_Y,
                   GAME_AREA_X + mapWidth, GAME_AREA_Y + borderThickness);

    // 下边框
    solidrectangle(GAME_AREA_X, GAME_AREA_Y + mapHeight - borderThickness,
                   GAME_AREA_X + mapWidth, GAME_AREA_Y + mapHeight);

    // 左边框
    solidrectangle(GAME_AREA_X, GAME_AREA_Y,
                   GAME_AREA_X + borderThickness, GAME_AREA_Y + mapHeight);

    // 右边框
    solidrectangle(GAME_AREA_X + mapWidth - borderThickness, GAME_AREA_Y,
                   GAME_AREA_X + mapWidth, GAME_AREA_Y + mapHeight);
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

    // 加速食物添加内部小圆作为标记
    if (food.type == SPEED_UP)
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
void Renderer::DrawUI(int score, int highScore, int length, int lives, int time, int wallCollisions, GameMode mode)
{
    // ========== 1. 绘制游戏区边框 (#112D4E深藏青色) ==========
    setlinecolor(RGB(17, 45, 78)); // #112D4E 深藏青
    setlinestyle(PS_SOLID, 3);     // 3px宽度
    rectangle(GAME_AREA_X, GAME_AREA_Y,
              GAME_AREA_X + GAME_AREA_WIDTH,
              GAME_AREA_Y + GAME_AREA_HEIGHT);

    // ========== 2. 绘制侧边栏背景 (#DBE2EF灰蓝色铺满) ==========
    setfillcolor(RGB(219, 226, 239)); // #DBE2EF 灰蓝
    setlinecolor(RGB(219, 226, 239));
    setlinestyle(PS_SOLID, 1);
    solidrectangle(SIDEBAR_X, 0, 1920, 1080); // 铺满右侧1/4

    setbkmode(TRANSPARENT);

    // 判断是否为多人对战模式
    bool isMultiplayerMode = (mode == LOCAL_PVP || mode == NET_PVP || mode == PVE);

    // =========================
    // 第一部分：对战信息 (Y: 60-200)
    // =========================

    // 汉字标题："对战信息" 居中
    LOGFONT titleFont;
    gettextstyle(&titleFont);
    titleFont.lfHeight = 32;
    titleFont.lfWeight = FW_BOLD;
    wcscpy_s(titleFont.lfFaceName, L"微软雅黑");
    titleFont.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&titleFont);
    settextcolor(RGB(63, 114, 175)); // #3F72AF 亮蓝

    const wchar_t *statusTitle = L"对战信息";
    int titleWidth = textwidth(statusTitle);
    outtextxy(SIDEBAR_CENTER_X - titleWidth / 2, 80, statusTitle);

    // 玩家图标：左侧P1(你)，右侧CPU(机)
    int iconY = 140;
    int iconSize = 30;

    // 左侧 🟦 P1 (你) - x: 1580
    int p1X = 1580;
    setfillcolor(RGB(63, 114, 175)); // #3F72AF 蓝色
    setlinecolor(RGB(63, 114, 175));
    solidrectangle(p1X, iconY, p1X + iconSize, iconY + iconSize);

    settextstyle(24, 0, L"微软雅黑");
    settextcolor(RGB(17, 45, 78)); // #112D4E 深藏青
    outtextxy(p1X + 40, iconY + 5, L"P1 (你)");

    // 右侧 🟥 CPU (机) - x: 1780
    int p2X = 1780;
    setfillcolor(RGB(224, 133, 133)); // #E08585 粉红色
    setlinecolor(RGB(224, 133, 133));
    solidrectangle(p2X, iconY, p2X + iconSize, iconY + iconSize);

    const wchar_t *p2Label = (mode == PVE) ? L"CPU (机)" : L"P2";
    outtextxy(p2X + 40, iconY + 5, p2Label);

    // =========================
    // 第二部分：核心数据 (Y: 250-550)
    // =========================

    wchar_t buffer[100];
    LOGFONT scoreFont, timeFont, labelFont;

    // 汉字标题："当前得分" (Y: 280)
    gettextstyle(&labelFont);
    labelFont.lfHeight = 24;
    labelFont.lfWeight = FW_NORMAL;
    wcscpy_s(labelFont.lfFaceName, L"微软雅黑");
    labelFont.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&labelFont);
    settextcolor(RGB(63, 114, 175)); // #3F72AF 亮蓝

    const wchar_t *scoreTitle = L"当前得分";
    int scoreTitleWidth = textwidth(scoreTitle);
    outtextxy(SIDEBAR_CENTER_X - scoreTitleWidth / 2, 280, scoreTitle);

    // 得分数值：150 (100px Arial Black, Y: 380)
    gettextstyle(&scoreFont);
    scoreFont.lfHeight = 100;
    scoreFont.lfWeight = FW_BOLD;
    wcscpy_s(scoreFont.lfFaceName, L"Arial Black");
    scoreFont.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&scoreFont);
    settextcolor(RGB(17, 45, 78)); // #112D4E 深藏青

    swprintf_s(buffer, L"%d", score);
    int scoreWidth = textwidth(buffer);
    outtextxy(SIDEBAR_CENTER_X - scoreWidth / 2, 330, buffer);

    // 汉字标题："游戏时间" (Y: 460)
    settextstyle(&labelFont);
    settextcolor(RGB(63, 114, 175));
    const wchar_t *timeTitle = L"游戏时间";
    int timeTitleWidth = textwidth(timeTitle);
    outtextxy(SIDEBAR_CENTER_X - timeTitleWidth / 2, 460, timeTitle);

    // 时间数值：02:14 (48px, Y: 520)
    gettextstyle(&timeFont);
    timeFont.lfHeight = 48;
    timeFont.lfWeight = FW_BOLD;
    wcscpy_s(timeFont.lfFaceName, L"Arial Black");
    timeFont.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&timeFont);
    settextcolor(RGB(17, 45, 78));

    int minutes = time / 60;
    int seconds = time % 60;
    swprintf_s(buffer, L"%02d:%02d", minutes, seconds);
    int timeWidth = textwidth(buffer);
    outtextxy(SIDEBAR_CENTER_X - timeWidth / 2, 510, buffer);

    // =========================
    // 第三块：食物图例 (Y: 650-850) - 双列布局
    // =========================
    DrawFoodLegendDualColumn();

    // =========================
    // 第四块：退出按钮（仅单人模式显示）
    // =========================
    if (!isMultiplayerMode && exitButton)
    {
        exitButton->Draw();
    }
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

    // 绘制返回按钮（等间距布局：新纪录底部+80px）
    int buttonX = (windowWidth - 400) / 2; // 居中
    int buttonY = windowHeight / 2 + 136;  // 540+56+80=676，等间距设计
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

    // 提示信息（按钮底部+80px）
    DrawTextCentered(L"点击按钮或按回车返回", buttonY + buttonHeight + 80, 36, RGB(17, 45, 78));
}

void Renderer::DrawMultiplayerGameOverScreen(bool playerWon, int p1Score, int p2Score, int p1Time, int p2Time)
{
    // 半透明背景
    setfillcolor(RGB(219, 226, 239)); // #DBE2EF
    solidrectangle(0, 0, windowWidth, windowHeight);

    // 游戏结束标题（不判定胜负）
    DrawTextCentered(L"游戏结束", windowHeight / 2 - 280, 120, RGB(17, 45, 78)); // #112D4E
    DrawTextCentered(L"感谢游玩！", windowHeight / 2 - 160, 64, RGB(17, 45, 78));

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

    // 绘制返回按钮（等间距布局：P2时长底部+80px）
    int buttonX = (windowWidth - 400) / 2; // 居中
    int buttonY = windowHeight / 2 + 216;  // centerY(500)+180+36+80=796，等间距设计
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
    const wchar_t *btnText = L"返回大厅";
    int textWidth = textwidth(btnText);
    int textHeight = textheight(btnText);
    int textX = buttonX + (buttonWidth - textWidth) / 2;
    int textY = buttonY + (buttonHeight - textHeight) / 2;
    outtextxy(textX, textY, btnText);

    // 提示信息（按钮底部+80px）
    DrawTextCentered(L"点击按钮或按回车返回大厅", buttonY + buttonHeight + 80, 36, RGB(17, 45, 78));
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
    return GAME_AREA_X + gridX * BLOCK_SIZE; // 加上左侧偏移
}

int Renderer::GridToPixelY(int gridY) const
{
    return GAME_AREA_Y + gridY * BLOCK_SIZE; // 加上顶部偏移
}

COLORREF Renderer::GetFoodColor(FoodType type) const
{
    switch (type)
    {
    case NORMAL_FOOD:
        return RGB(247, 197, 72); // #F7C548 金黄色
    case SPEED_UP:
        return RGB(255, 107, 107); // #FF6B6B 红色（加速）
    default:
        return RGB(247, 197, 72);
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
        setlinecolor(RGB(249, 247, 247));                                                      // #F9F7F7 背景色作为边框，制造间隙感
        setlinestyle(PS_SOLID, 2);                                                             // 2px边框宽度
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
        setlinecolor(RGB(249, 247, 247)); // #F9F7F7 背景色边框
        setlinestyle(PS_SOLID, 2);
        fillroundrect(pixelX + 2, pixelY + 2, pixelX + BLOCK_SIZE + 1, pixelY + BLOCK_SIZE + 1, 8, 8);

        // 再绘制主体（带背景色边框）
        setfillcolor(color);
        setlinecolor(RGB(249, 247, 247)); // #F9F7F7 背景色边框，制造间隙感
        setlinestyle(PS_SOLID, 2);
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
    if (exitButton)
    {
        x = exitButton->GetX();
        y = exitButton->GetY();
        width = exitButton->GetWidth();
        height = exitButton->GetHeight();
    }
    else
    {
        // 默认值（兼容）
        int dividerX = MAP_WIDTH * BLOCK_SIZE;
        int sidebarWidth = windowWidth - dividerX;
        width = (int)(sidebarWidth * 0.7);
        height = 70;
        x = dividerX + (sidebarWidth - width) / 2;
        y = windowHeight - 150;
    }
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

// ============== 食物图例绘制 ==============
void Renderer::DrawFoodLegend(int startY)
{
    int dividerX = MAP_WIDTH * BLOCK_SIZE;
    int uiX = dividerX + 60;
    int currentY = startY;
    int itemHeight = 60;
    int iconSize = 28;
    int padding = 20;

    // 计算背景矩形尺寸
    int bgX = uiX - padding;
    int bgY = currentY - padding;
    int bgWidth = (windowWidth - dividerX) - 80;         // 留出右侧边距
    int bgHeight = 50 + 50 + (itemHeight * 5) + padding; // 标题+间距+5个食物项+底部边距

    // 绘制白色圆角矩形背景
    setfillcolor(RGB(255, 255, 255)); // 白色
    setlinecolor(RGB(219, 226, 239)); // #DBE2EF 浅边框
    setlinestyle(PS_SOLID, 2);
    fillroundrect(bgX, bgY, bgX + bgWidth, bgY + bgHeight, 20, 20);

    // 标题
    settextstyle(32, 0, L"微软雅黑");
    settextcolor(RGB(63, 114, 175)); // #3F72AF
    setbkmode(TRANSPARENT);
    outtextxy(uiX, currentY, L"食物图例");
    currentY += 50;

    // 设置字体
    settextstyle(24, 0, L"微软雅黑");

    // 定义食物类型、颜色和说明
    struct FoodLegendItem
    {
        COLORREF color;
        const wchar_t *name;
        const wchar_t *desc;
    };

    FoodLegendItem items[] = {
        {RGB(247, 197, 72), L"普通", L"+10分"},        // #F7C548 金黄色
        {RGB(255, 107, 107), L"加分", L"+20分"},       // #FF6B6B 珊瑚粉
        {RGB(54, 209, 220), L"精灵果", L"+50分"},      // #36D1DC 青碧色
        {RGB(74, 105, 133), L"恶果", L"-100分"},       // #4A6985 深灰蓝
        {RGB(219, 226, 239), L"变大果", L"+2格-10分"}, // #DBE2EF 灰蓝
    };

    for (const auto &item : items)
    {
        // 绘制食物图标（圆点）
        setfillcolor(item.color);
        setlinecolor(item.color);
        solidcircle(uiX + 14, currentY + 14, iconSize / 2);

        // 绘制食物名称
        settextcolor(RGB(17, 45, 78)); // #112D4E
        outtextxy(uiX + 45, currentY + 2, item.name);

        // 绘制说明（右侧）
        settextcolor(RGB(63, 114, 175)); // 浅色
        int nameWidth = textwidth(item.name);
        outtextxy(uiX + 45 + nameWidth + 15, currentY + 2, item.desc);

        currentY += itemHeight;
    }
}

// ============== 食物图例绘制（双列布局）==============
void Renderer::DrawFoodLegendDualColumn()
{
    // =========================
    // 第三部分：食物图例 (Y: 600-850)
    // =========================

    // 汉字标题："食物图例" (Y: 620)
    LOGFONT titleFont;
    gettextstyle(&titleFont);
    titleFont.lfHeight = 24;
    titleFont.lfWeight = FW_NORMAL;
    wcscpy_s(titleFont.lfFaceName, L"微软雅黑");
    titleFont.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&titleFont);
    settextcolor(RGB(63, 114, 175)); // #3F72AF 亮蓝
    setbkmode(TRANSPARENT);

    const wchar_t *legendTitle = L"食物图例";
    int legendTitleWidth = textwidth(legendTitle);
    outtextxy(SIDEBAR_CENTER_X - legendTitleWidth / 2, 620, legendTitle);

    // 定义食物项 (只保留2项)
    struct FoodItem
    {
        COLORREF color;
        const wchar_t *name;
    };

    FoodItem foods[] = {
        {RGB(247, 197, 72), L"普通"},  // #F7C548 金黄色
        {RGB(255, 107, 107), L"加速"}, // #FF6B6B 红色
    };

    // 双列布局基准坐标
    int leftX = 1520;    // 左列X
    int rightX = 1720;   // 右列X
    int startY = 680;    // 起始Y
    int rowHeight = 60;  // 行距
    int iconRadius = 12; // 半径12px

    // 设置字体 (22px深色)
    settextstyle(22, 0, L"微软雅黑");
    settextcolor(RGB(17, 45, 78)); // #112D4E 深藏青

    // 绘制2个食物项（双列排列）
    for (int i = 0; i < 2; i++)
    {
        int posX, posY;

        // 双列排列
        posX = (i % 2 == 0) ? leftX : rightX; // 左右列交替
        posY = startY + (i / 2) * rowHeight;  // 每两个换行

        // 绘制圆形图标
        setfillcolor(foods[i].color);
        setlinecolor(foods[i].color);
        solidcircle(posX + iconRadius, posY + iconRadius, iconRadius);

        // 绘制食物名称
        outtextxy(posX + iconRadius * 2 + 10, posY + 3, foods[i].name);
    }
}
