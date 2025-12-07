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

    // 单人游戏（进入子菜单）
    MenuItem item1;
    item1.text = L"单人游戏";
    item1.mode = SINGLE;
    item1.isSubmenu = true;
    item1.isExit = false;
    menuItems.push_back(item1);

    // 双人游戏（进入子菜单）
    MenuItem item2;
    item2.text = L"双人游戏";
    item2.mode = LOCAL_PVP;
    item2.isSubmenu = true;
    item2.isExit = false;
    menuItems.push_back(item2);

    // 历史记录
    MenuItem item3;
    item3.text = L"历史记录";
    item3.mode = SINGLE;
    item3.isSubmenu = false;
    item3.isExit = false;
    item3.isHistory = true;
    menuItems.push_back(item3);

    // 退出游戏
    MenuItem item4;
    item4.text = L"退出游戏";
    item4.mode = EXIT;
    item4.isSubmenu = false;
    item4.isExit = true;
    menuItems.push_back(item4);

    CreateButtons();
}

void MenuScene::InitSinglePlayerMenu()
{
    menuItems.clear();

    // 入门版
    MenuItem item1;
    item1.text = L"入门版";
    item1.mode = BEGINNER;
    item1.isSubmenu = false;
    item1.isExit = false;
    menuItems.push_back(item1);

    // 进阶版
    MenuItem item2;
    item2.text = L"进阶版";
    item2.mode = ADVANCED;
    item2.isSubmenu = false;
    item2.isExit = false;
    menuItems.push_back(item2);

    // 高级版
    MenuItem item3;
    item3.text = L"高级版";
    item3.mode = EXPERT;
    item3.isSubmenu = false;
    item3.isExit = false;
    menuItems.push_back(item3);

    // 返回
    MenuItem item4;
    item4.text = L"返回";
    item4.mode = SINGLE;
    item4.isSubmenu = false;
    item4.isExit = true;
    menuItems.push_back(item4);

    CreateButtons();
}

void MenuScene::InitMultiplayerMenu()
{
    menuItems.clear();

    // 本地双人
    MenuItem item1;
    item1.text = L"本地双人";
    item1.mode = LOCAL_PVP;
    item1.isSubmenu = false;
    item1.isExit = false;
    menuItems.push_back(item1);

    // 人机对战
    MenuItem item2;
    item2.text = L"人机对战";
    item2.mode = PVE;
    item2.isSubmenu = false;
    item2.isExit = false;
    menuItems.push_back(item2);

    // 在线双人
    MenuItem item3;
    item3.text = L"在线双人";
    item3.mode = NET_PVP;
    item3.isSubmenu = false;
    item3.isExit = false;
    menuItems.push_back(item3);

    // 返回
    MenuItem item4;
    item4.text = L"返回";
    item4.mode = SINGLE;
    item4.isSubmenu = true;
    item4.isExit = false;
    menuItems.push_back(item4);

    CreateButtons();
}

void MenuScene::CreateButtons()
{
    buttons.clear();

    // 计算布局参数
    int startX = 660;                                            // 1920/2 - 300 居中
    int startY = (currentMenu == SINGLEPLAYER_MENU) ? 270 : 360; // 单人菜单靠上
    int itemWidth = 600;
    int itemHeight = 100;
    int spacing = (currentMenu == MAIN_MENU) ? 160 : 140; // 主菜单间距更大

    // 为每个菜单项创建按钮
    for (size_t i = 0; i < menuItems.size(); ++i)
    {
        int btnY = startY + static_cast<int>(i) * spacing;
        auto btn = std::make_unique<UIButton>(startX, btnY, itemWidth, itemHeight,
                                              menuItems[i].text, BUTTON_CAPSULE);

        // 设置颜色（统一的莫兰迪配色）
        btn->SetColors(RGB(219, 226, 239), RGB(85, 132, 188), RGB(63, 114, 175));
        btn->SetTextColor(RGB(17, 45, 78), RGB(255, 255, 255));
        btn->SetFontSize(48);

        buttons.push_back(std::move(btn));
    }
}

void MenuScene::Render()
{
    BeginBatchDraw();
    cleardevice();

    // 绘制标题
    DrawTitle();

    // 绘制所有按钮
    for (size_t i = 0; i < buttons.size(); ++i)
    {
        // 更新按钮状态（根据是否选中）
        if (i == static_cast<size_t>(selectedOption))
        {
            // 使用键盘选中时，强制设为HOVER状态
            buttons[i]->UpdateState(buttons[i]->GetX() + 1, buttons[i]->GetY() + 1, false);
        }
        else
        {
            buttons[i]->UpdateState(-1, -1, false); // NORMAL状态
        }

        buttons[i]->Draw();
    }

    // 绘制说明文字
    DrawInstructions();

    EndBatchDraw();
}

void MenuScene::HandleMouseInput()
{
    MOUSEMSG msg;
    while (inputMgr.GetNextMouseMessage(msg))
    {
        // 检测鼠标移动 - 使用UIButton的IsMouseOver
        if (msg.uMsg == WM_MOUSEMOVE)
        {
            for (size_t i = 0; i < buttons.size(); ++i)
            {
                if (buttons[i]->IsMouseOver(msg.x, msg.y))
                {
                    selectedOption = static_cast<int>(i);
                    break;
                }
            }
        }

        // 检测鼠标点击
        if (msg.uMsg == WM_LBUTTONDOWN)
        {
            for (size_t i = 0; i < buttons.size(); ++i)
            {
                if (buttons[i]->IsMouseOver(msg.x, msg.y))
                {
                    selectedOption = static_cast<int>(i);
                    MenuItem &item = menuItems[selectedOption];

                    if (item.isHistory)
                    {
                        ShowHistoryScene();
                    }
                    else if (item.isExit)
                    {
                        if (currentMenu == MAIN_MENU)
                        {
                            exit(0);
                        }
                        else
                        {
                            currentMenu = MAIN_MENU;
                            InitMainMenu();
                            selectedOption = 0;
                        }
                    }
                    else if (item.isSubmenu)
                    {
                        if (currentMenu == MAIN_MENU)
                        {
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
