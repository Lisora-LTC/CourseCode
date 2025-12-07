#include <winsock2.h>
#include "MenuScene.h"
#include "HistoryScene.h"
#include <windows.h>

// ============== 构造与析构 ==============
MenuScene::MenuScene(bool manageWindow)
    : selectedOption(0), menuRunning(false), currentMenu(MAIN_MENU), manageWindow(manageWindow)
{
    InitMainMenu();
}

MenuScene::~MenuScene()
{
    // 只有当MenuScene管理窗口时才关闭
    if (manageWindow)
    {
        closegraph();
    }
}

// ============== 主方法 ==============
GameMode MenuScene::Show()
{
    // 1. 初始化图形窗口（仅当管理窗口时）
    if (manageWindow)
    {
        initgraph(1920, 1080);
    }

    setbkcolor(RGB(249, 247, 247)); // #F9F7F7
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

        // 更新输入缓冲
        inputMgr.Update();

        HandleMouseInput();
        HandleKeyboardInput();
        Render();
        Sleep(10);
    }

    // 返回选择的模式
    selectedMode = menuItems[selectedOption].mode;

    // 只有当管理窗口时才关闭
    if (manageWindow)
    {
        closegraph();
    }

    return selectedMode;
}

// ============== 私有方法 ==============
void MenuScene::InitMainMenu()
{
    menuItems.clear();

    int startX = 660;     // 1920/2 - 300 居中
    int startY = 360;     // 从上往下1/3处
    int itemWidth = 600;  // 放大到2倍
    int itemHeight = 100; // 放大到2倍
    int spacing = 160;    // 增加间距从140增加到160，增加呼吸感

    // 单人游戏（进入子菜单）
    MenuItem item1;
    item1.text = L"单人游戏";
    item1.mode = SINGLE; // 默认值
    item1.x = startX;
    item1.y = startY;
    item1.width = itemWidth;
    item1.height = itemHeight;
    item1.isSubmenu = true; // 修改为进入子菜单
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

    // 历史记录
    MenuItem item3;
    item3.text = L"历史记录";
    item3.mode = SINGLE; // 不使用
    item3.x = startX;
    item3.y = startY + spacing * 2;
    item3.width = itemWidth;
    item3.height = itemHeight;
    item3.isSubmenu = false;
    item3.isExit = false;
    item3.isHistory = true;
    menuItems.push_back(item3);

    // 退出游戏
    MenuItem item4;
    item4.text = L"退出游戏";
    item4.mode = EXIT; // 使用EXIT模式
    item4.x = startX;
    item4.y = startY + spacing * 3;
    item4.width = itemWidth;
    item4.height = itemHeight;
    item4.isSubmenu = false;
    item4.isExit = true;
    menuItems.push_back(item4);
}

void MenuScene::InitSinglePlayerMenu()
{
    menuItems.clear();

    int startX = 660;     // 居中
    int startY = 270;     // 靠上一些
    int itemWidth = 600;  // 放大到2倍
    int itemHeight = 100; // 放大到2倍
    int spacing = 140;    // 放大到2倍

    // 入门版
    MenuItem item1;
    item1.text = L"入门版";
    item1.mode = BEGINNER;
    item1.x = startX;
    item1.y = startY;
    item1.width = itemWidth;
    item1.height = itemHeight;
    item1.isSubmenu = false;
    item1.isExit = false;
    menuItems.push_back(item1);

    // 进阶版
    MenuItem item2;
    item2.text = L"进阶版";
    item2.mode = ADVANCED;
    item2.x = startX;
    item2.y = startY + spacing;
    item2.width = itemWidth;
    item2.height = itemHeight;
    item2.isSubmenu = false;
    item2.isExit = false;
    menuItems.push_back(item2);

    // 高级版
    MenuItem item3;
    item3.text = L"高级版";
    item3.mode = EXPERT;
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
    item4.isSubmenu = false;
    item4.isExit = true; // 标记为返回
    menuItems.push_back(item4);
}

void MenuScene::InitMultiplayerMenu()
{
    menuItems.clear();

    int startX = 660;     // 居中
    int startY = 360;     // 从上往下1/3处
    int itemWidth = 600;  // 放大到2倍
    int itemHeight = 100; // 放大到2倍
    int spacing = 140;    // 放大到2倍

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
    // 定义颜色变量
    COLORREF btnColor;
    COLORREF textColor;
    int radius = item.height / 2;

    // 统一逻辑：所有按钮未悬停时为灰蓝色，悬停时变为亮蓝色
    if (isSelected)
    {
        // 悬停状态：亮蓝色，模拟发光效果
        btnColor = RGB(85, 132, 188);   // #5584BC
        textColor = RGB(255, 255, 255); // #FFFFFF 纯白
    }
    else
    {
        // 常态：灰蓝色
        btnColor = RGB(219, 226, 239); // #DBE2EF
        textColor = RGB(17, 45, 78);   // #112D4E 深藏青
    }

    // 使用拼图法绘制胶囊形按钮（两个圆+一个矩形）
    setfillcolor(btnColor);
    setlinecolor(btnColor);                                                                      // 边框与填充色一致，消除黑边
    solidcircle(item.x + radius, item.y + radius, radius);                                       // 左半圆
    solidcircle(item.x + item.width - radius, item.y + radius, radius);                          // 右半圆
    solidrectangle(item.x + radius, item.y, item.x + item.width - radius, item.y + item.height); // 中间矩形

    // 绘制文字
    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = 48;      // 所有按钮统一使用48px字体
    f.lfWeight = FW_BOLD; // 加粗
    wcscpy_s(f.lfFaceName, L"微软雅黑");
    f.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&f);
    settextcolor(textColor);
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
    // 使用GetNextMouseMessage从缓冲区读取消息
    while (inputMgr.GetNextMouseMessage(msg))
    {
        // 检测鼠标移动
        if (msg.uMsg == WM_MOUSEMOVE)
        {
            bool foundHover = false;
            for (size_t i = 0; i < menuItems.size(); ++i)
            {
                if (IsMouseOver(menuItems[i], msg.x, msg.y))
                {
                    selectedOption = static_cast<int>(i);
                    foundHover = true;
                    break;
                }
            }
            // 如果鼠标不在任何按钮上，不改变选中状态
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

                    if (item.isHistory)
                    {
                        // 显示历史记录场景
                        ShowHistoryScene();
                    }
                    else if (item.isExit)
                    {
                        if (currentMenu == MAIN_MENU)
                        {
                            // 主菜单的“退出游戏”：直接退出程序
                            exit(0);
                        }
                        else
                        {
                            // 子菜单的“返回”：返回主菜单
                            currentMenu = MAIN_MENU;
                            InitMainMenu();
                            selectedOption = 0;
                        }
                    }
                    else if (item.isSubmenu)
                    {
                        if (currentMenu == MAIN_MENU)
                        {
                            // 进入子菜单
                            if (item.mode == SINGLE)
                            {
                                currentMenu = SINGLEPLAYER_MENU;
                                InitSinglePlayerMenu();
                            }
                            else
                            {
                                currentMenu = MULTIPLAYER_MENU;
                                InitMultiplayerMenu();
                            }
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

        if (item.isHistory)
        {
            // 显示历史记录场景
            ShowHistoryScene();
        }
        else if (item.isExit)
        {
            if (currentMenu == MAIN_MENU)
            {
                // 主菜单的“退出游戏”：直接退出程序
                exit(0);
            }
            else
            {
                // 子菜单的“返回”：返回主菜单
                currentMenu = MAIN_MENU;
                InitMainMenu();
                selectedOption = 0;
            }
        }
        else if (item.isSubmenu)
        {
            if (currentMenu == MAIN_MENU)
            {
                // 进入子菜单
                if (item.mode == SINGLE)
                {
                    currentMenu = SINGLEPLAYER_MENU;
                    InitSinglePlayerMenu();
                }
                else
                {
                    currentMenu = MULTIPLAYER_MENU;
                    InitMultiplayerMenu();
                }
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
    settextstyle(120, 0, L"微软雅黑");
    settextcolor(RGB(17, 45, 78)); // #112D4E 深蓝色标题
    setbkmode(TRANSPARENT);

    const wchar_t *title;
    if (currentMenu == MAIN_MENU)
        title = L"贪吃蛇游戏";
    else if (currentMenu == SINGLEPLAYER_MENU)
        title = L"选择单人模式";
    else
        title = L"选择双人模式";

    int textWidth = textwidth(title);
    outtextxy((1920 - textWidth) / 2, 120, title); // 调整为1920居中
}

void MenuScene::DrawInstructions()
{
    settextstyle(32, 0, L"微软雅黑");
    settextcolor(RGB(17, 45, 78)); // #112D4E 深蓝色标题
    setbkmode(TRANSPARENT);

    const wchar_t *instruction = L"使用上下方向键或鼠标选择，回车确认";
    int textWidth = textwidth(instruction);
    outtextxy((1920 - textWidth) / 2, 980, instruction); // 放在右下角靠上
}

void MenuScene::ShowHistoryScene()
{
    // 历史记录场景不管理窗口，只使用当前窗口
    HistoryScene historyScene(false);
    historyScene.Show();

    // 返回菜单之后，重新清屏
    setbkcolor(RGB(249, 247, 247));
    cleardevice();
}
