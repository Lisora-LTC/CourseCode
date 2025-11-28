#include "MenuScene.h"
#include <windows.h>

// ============== 构造与析构 ==============
MenuScene::MenuScene() : selectedOption(0), menuRunning(false)
{
    InitMenuItems();
}

MenuScene::~MenuScene()
{
    closegraph();
}

// ============== 主方法 ==============
GameMode MenuScene::Show()
{
    // 1. 初始化图形窗口
    initgraph(800, 600);
    setbkcolor(RGB(20, 20, 40));
    cleardevice();

    menuRunning = true;
    GameMode selectedMode = SINGLE;

    // 2. 进入菜单循环
    while (menuRunning)
    {
        HandleMouseInput();
        HandleKeyboardInput();
        Render();
        Sleep(50);
    }

    // 返回选择的模式
    selectedMode = menuItems[selectedOption].mode;
    closegraph();

    return selectedMode;
}

// ============== 私有方法 ==============
void MenuScene::InitMenuItems()
{
    menuItems.clear();

    int startX = 250;
    int startY = 180;
    int itemWidth = 300;
    int itemHeight = 50;
    int spacing = 60;

    // 单人模式
    MenuItem item1;
    item1.text = L"单人模式";
    item1.mode = SINGLE;
    item1.x = startX;
    item1.y = startY;
    item1.width = itemWidth;
    item1.height = itemHeight;
    menuItems.push_back(item1);

    // 本地双人
    MenuItem item2;
    item2.text = L"本地双人对战";
    item2.mode = LOCAL_PVP;
    item2.x = startX;
    item2.y = startY + spacing;
    item2.width = itemWidth;
    item2.height = itemHeight;
    menuItems.push_back(item2);

    // 入门版
    MenuItem item3;
    item3.text = L"入门版（撞墙死亡）";
    item3.mode = BEGINNER;
    item3.x = startX;
    item3.y = startY + spacing * 2;
    item3.width = itemWidth;
    item3.height = itemHeight;
    menuItems.push_back(item3);

    // 进阶版
    MenuItem item4;
    item4.text = L"进阶版（蛇尸变墙）";
    item4.mode = ADVANCED;
    item4.x = startX;
    item4.y = startY + spacing * 3;
    item4.width = itemWidth;
    item4.height = itemHeight;
    menuItems.push_back(item4);

    // 高级版
    MenuItem item5;
    item5.text = L"高级版（蛇尸变食物）";
    item5.mode = EXPERT;
    item5.x = startX;
    item5.y = startY + spacing * 4;
    item5.width = itemWidth;
    item5.height = itemHeight;
    menuItems.push_back(item5);
}

void MenuScene::Render()
{
    BeginBatchDraw();
    cleardevice();

    // 绘制标题
    DrawTitle();

    // 绘制所有菜单项
    for (size_t i = 0; i < menuItems.size(); ++i)
    {
        bool isSelected = (i == static_cast<size_t>(selectedOption));
        DrawMenuItem(menuItems[i], isSelected);
    }

    // 绘制说明文字
    DrawInstructions();

    EndBatchDraw();
}

void MenuScene::DrawMenuItem(const MenuItem &item, bool isSelected)
{
    // 绘制按钮背景
    if (isSelected)
    {
        setfillcolor(RGB(50, 150, 250));
        setlinecolor(RGB(100, 200, 255));
    }
    else
    {
        setfillcolor(RGB(50, 50, 80));
        setlinecolor(RGB(100, 100, 150));
    }

    setlinestyle(PS_SOLID, 2);
    fillrectangle(item.x, item.y, item.x + item.width, item.y + item.height);

    // 绘制文字
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);

    int textWidth = textwidth(item.text.c_str());
    int textHeight = textheight(item.text.c_str());
    int textX = item.x + (item.width - textWidth) / 2;
    int textY = item.y + (item.height - textHeight) / 2;

    outtextxy(textX, textY, item.text.c_str());
}

void MenuScene::HandleMouseInput()
{
    if (MouseHit())
    {
        MOUSEMSG msg = GetMouseMsg();

        // 检测鼠标移动
        if (msg.uMsg == WM_MOUSEMOVE)
        {
            for (size_t i = 0; i < menuItems.size(); ++i)
            {
                if (IsMouseOver(menuItems[i], msg.x, msg.y))
                {
                    selectedOption = static_cast<int>(i);
                    break;
                }
            }
        }

        // 检测鼠标点击
        if (msg.uMsg == WM_LBUTTONDOWN)
        {
            for (size_t i = 0; i < menuItems.size(); ++i)
            {
                if (IsMouseOver(menuItems[i], msg.x, msg.y))
                {
                    selectedOption = static_cast<int>(i);
                    menuRunning = false;
                    break;
                }
            }
        }
    }
}

void MenuScene::HandleKeyboardInput()
{
    // 上键
    if (GetAsyncKeyState(VK_UP) & 0x8000)
    {
        static bool upPressed = false;
        if (!upPressed)
        {
            selectedOption = (selectedOption - 1 + menuItems.size()) % menuItems.size();
            upPressed = true;
        }
    }
    else
    {
        static bool upPressed = false;
        upPressed = false;
    }

    // 下键
    if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
        static bool downPressed = false;
        if (!downPressed)
        {
            selectedOption = (selectedOption + 1) % menuItems.size();
            downPressed = true;
        }
    }
    else
    {
        static bool downPressed = false;
        downPressed = false;
    }

    // 回车键确认
    if (GetAsyncKeyState(VK_RETURN) & 0x8000)
    {
        static bool enterPressed = false;
        if (!enterPressed)
        {
            menuRunning = false;
            enterPressed = true;
        }
    }
    else
    {
        static bool enterPressed = false;
        enterPressed = false;
    }

    // ESC退出程序
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
        exit(0);
    }
}

bool MenuScene::IsMouseOver(const MenuItem &item, int mouseX, int mouseY)
{
    return mouseX >= item.x && mouseX <= item.x + item.width &&
           mouseY >= item.y && mouseY <= item.y + item.height;
}

void MenuScene::DrawTitle()
{
    settextstyle(60, 0, L"微软雅黑");
    settextcolor(RGB(255, 215, 0));
    setbkmode(TRANSPARENT);

    const wchar_t *title = L"🐍 贪吃蛇游戏";
    int textWidth = textwidth(title);
    outtextxy((800 - textWidth) / 2, 50, title);
}

void MenuScene::DrawInstructions()
{
    settextstyle(18, 0, L"微软雅黑");
    settextcolor(RGB(150, 150, 150));
    setbkmode(TRANSPARENT);

    outtextxy(180, 540, L"使用鼠标点击或键盘↑↓键选择，回车确认");
}
