#include <winsock2.h>
#include "MenuScene.h"
#include <windows.h>

// ============== 构造与析构 ==============
MenuScene::MenuScene()
    : selectedOption(0), menuRunning(false), currentMenu(MAIN_MENU)
{
    InitMainMenu();
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
        // 检测窗口是否被关闭
        if (inputMgr.IsWindowClosed())
        {
            exit(0);
        }

        HandleMouseInput();
        HandleKeyboardInput();
        Render();
        Sleep(10);
    }

    // 返回选择的模式
    selectedMode = menuItems[selectedOption].mode;
    closegraph();

    return selectedMode;
}

// ============== 私有方法 ==============
void MenuScene::InitMainMenu()
{
    menuItems.clear();

    int startX = 250;
    int startY = 200;
    int itemWidth = 300;
    int itemHeight = 50;
    int spacing = 70;

    // 单人游戏
    MenuItem item1;
    item1.text = L"单人游戏";
    item1.mode = SINGLE;
    item1.x = startX;
    item1.y = startY;
    item1.width = itemWidth;
    item1.height = itemHeight;
    item1.isSubmenu = false;
    item1.isExit = false;
    menuItems.push_back(item1);

    // 双人游戏（进入子菜单）
    MenuItem item2;
    item2.text = L"双人游戏";
    item2.mode = LOCAL_PVP; // 默认值
    item2.x = startX;
    item2.y = startY + spacing;
    item2.width = itemWidth;
    item2.height = itemHeight;
    item2.isSubmenu = true;
    item2.isExit = false;
    menuItems.push_back(item2);

    // 退出游戏
    MenuItem item3;
    item3.text = L"退出游戏";
    item3.mode = SINGLE; // 不使用
    item3.x = startX;
    item3.y = startY + spacing * 2;
    item3.width = itemWidth;
    item3.height = itemHeight;
    item3.isSubmenu = false;
    item3.isExit = true;
    menuItems.push_back(item3);
}

void MenuScene::InitMultiplayerMenu()
{
    menuItems.clear();

    int startX = 250;
    int startY = 200;
    int itemWidth = 300;
    int itemHeight = 50;
    int spacing = 70;

    // 本地双人
    MenuItem item1;
    item1.text = L"本地双人";
    item1.mode = LOCAL_PVP;
    item1.x = startX;
    item1.y = startY;
    item1.width = itemWidth;
    item1.height = itemHeight;
    item1.isSubmenu = false;
    item1.isExit = false;
    menuItems.push_back(item1);

    // 人机对战
    MenuItem item2;
    item2.text = L"人机对战";
    item2.mode = PVE;
    item2.x = startX;
    item2.y = startY + spacing;
    item2.width = itemWidth;
    item2.height = itemHeight;
    item2.isSubmenu = false;
    item2.isExit = false;
    menuItems.push_back(item2);

    // 在线双人
    MenuItem item3;
    item3.text = L"在线双人";
    item3.mode = NET_PVP;
    item3.x = startX;
    item3.y = startY + spacing * 2;
    item3.width = itemWidth;
    item3.height = itemHeight;
    item3.isSubmenu = false;
    item3.isExit = false;
    menuItems.push_back(item3);

    // 返回
    MenuItem item4;
    item4.text = L"返回";
    item4.mode = SINGLE; // 不使用
    item4.x = startX;
    item4.y = startY + spacing * 3;
    item4.width = itemWidth;
    item4.height = itemHeight;
    item4.isSubmenu = true; // 标记为特殊项
    item4.isExit = false;
    menuItems.push_back(item4);
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
    MOUSEMSG msg;
    while (inputMgr.GetLatestMouseMessage(msg))
    {
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
                    MenuItem &item = menuItems[selectedOption];

                    if (item.isExit)
                    {
                        exit(0);
                    }
                    else if (item.isSubmenu)
                    {
                        if (currentMenu == MAIN_MENU)
                        {
                            // 进入双人游戏子菜单
                            currentMenu = MULTIPLAYER_MENU;
                            InitMultiplayerMenu();
                            selectedOption = 0;
                        }
                        else
                        {
                            // 返回主菜单
                            currentMenu = MAIN_MENU;
                            InitMainMenu();
                            selectedOption = 0;
                        }
                    }
                    else
                    {
                        menuRunning = false;
                    }
                    break;
                }
            }
        }
    }
}

void MenuScene::HandleKeyboardInput()
{
    // 上键 - 边沿触发（自动防抖）
    if (inputMgr.IsKeyJustPressed(VK_UP))
    {
        selectedOption = (int)((selectedOption - 1 + menuItems.size()) % menuItems.size());
    }

    // 下键 - 边沿触发（自动防抖）
    if (inputMgr.IsKeyJustPressed(VK_DOWN))
    {
        selectedOption = (int)((selectedOption + 1) % menuItems.size());
    }

    // 回车键确认 - 边沿触发（自动防抖）
    if (inputMgr.IsKeyJustPressed(VK_RETURN))
    {
        MenuItem &item = menuItems[selectedOption];

        if (item.isExit)
        {
            exit(0);
        }
        else if (item.isSubmenu)
        {
            if (currentMenu == MAIN_MENU)
            {
                // 进入双人游戏子菜单
                currentMenu = MULTIPLAYER_MENU;
                InitMultiplayerMenu();
                selectedOption = 0;
            }
            else
            {
                // 返回主菜单
                currentMenu = MAIN_MENU;
                InitMainMenu();
                selectedOption = 0;
            }
        }
        else
        {
            menuRunning = false;
        }
    }

    // ESC退出程序 - 边沿触发（自动防抖）
    if (inputMgr.IsKeyJustPressed(VK_ESCAPE))
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

    const wchar_t *title = currentMenu == MAIN_MENU ? L"贪吃蛇游戏" : L"选择游戏模式";
    int textWidth = textwidth(title);
    outtextxy((800 - textWidth) / 2, 80, title);
}

void MenuScene::DrawInstructions()
{
    settextstyle(18, 0, L"微软雅黑");
    settextcolor(RGB(150, 150, 150));
    setbkmode(TRANSPARENT);

    outtextxy(180, 540, L"使用鼠标点击或键盘↑↓键选择，回车确认");
}
