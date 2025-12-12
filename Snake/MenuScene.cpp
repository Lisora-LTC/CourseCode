#include <winsock2.h>
#include "MenuScene.h"
#include "HistoryScene.h"
#include "NetworkModeScene.h"
#include "RoomListScene.h"
#include "LobbyScene.h"
#include "NetworkManager.h"
#include "GameManager.h"
#include "Snake.h"
#include "AIController.h"
#include "GameMap.h"
#include "FoodManager.h"
#include "KeyboardController.h"
#include "Renderer.h"

// 辅助函数：将 wstring 转换为 UTF-8 编码的 string
static std::string WstringToUtf8(const std::wstring &wstr)
{
    if (wstr.empty())
        return std::string();

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
    return strTo;
}
#include <windows.h>
#include <ctime>

// ============== 构造与析构 ==============
MenuScene::MenuScene(bool manageWindow)
    : selectedOption(0), menuRunning(false), currentMenu(MAIN_MENU), manageWindow(manageWindow),
      backgroundSnake1(nullptr), backgroundSnake2(nullptr),
      backgroundAI1(nullptr), backgroundAI2(nullptr),
      backgroundMap1(nullptr), backgroundMap2(nullptr),
      backgroundFood1(nullptr), backgroundFood2(nullptr),
      backgroundRenderer(nullptr), backgroundUpdateCounter(0),
      networkMgr(nullptr), playerName(L"Player")
{
    srand(static_cast<unsigned int>(time(nullptr))); // 初始化随机数生成器
    InitMainMenu();
    InitBackgroundSnake();
}

MenuScene::~MenuScene()
{
    CleanupBackgroundSnake();

    // 清理网络管理器
    if (networkMgr)
    {
        delete networkMgr;
        networkMgr = nullptr;
    }

    // 只有当MenuScene管理窗口时才关闭
    if (manageWindow)
    {
        closegraph();
    }
}

// ============== 主方法 ==============
GameMode MenuScene::Show()
{
    // 1. 初始化图形窗口（仅当管理窗口时）- 使用1920x1080全屏
    if (manageWindow)
    {
        initgraph(WINDOW_WIDTH, WINDOW_HEIGHT); // 1920x1080
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

        // 更新背景蛇
        UpdateBackgroundSnake();

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

    // 绘制背景蛇（在所有UI元素之前）
    RenderBackgroundSnake();

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
                    else if (item.mode == NET_PVP)
                    {
                        // 进入网络对战模式
                        if (HandleNetworkMode())
                        {
                            menuRunning = false;
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
        else if (item.mode == NET_PVP)
        {
            // 进入网络对战模式
            if (HandleNetworkMode())
            {
                menuRunning = false;
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

    const wchar_t *instruction = L"使用鼠标选择";
    int textWidth = textwidth(instruction);
    outtextxy((1920 - textWidth) / 2, 980, instruction); // 放在右下角靠上
}

void MenuScene::ShowHistoryScene()
{
    // 历史记录场景不管理窗口，只使用当前窗口
    HistoryScene historyScene(false);
    historyScene.Show();

    // 返回菜单之后，重新清屏并重置背景蛇计数器
    setbkcolor(RGB(249, 247, 247));
    cleardevice();
    backgroundUpdateCounter = 0; // 重置计数器，避免立即更新
}

// ============== 背景蛇系统实现 ==============
void MenuScene::InitBackgroundSnake()
{
    // 1. 创建两个简化地图（只有边界，无内墙）
    // 左侧地图：0-21列（按钮startX=660, 660/30=22，所以P1在x≤21）
    backgroundMap1 = new GameMap(22, 36); // P1活动区域
    backgroundMap1->InitBoundaryWalls();

    // 右侧地图：42-63列（按钮endX=1260, 1260/30=42，所以P2在x≥42）
    backgroundMap2 = new GameMap(22, 36); // P2活动区域（42-63共22格）
    backgroundMap2->InitBoundaryWalls();

    // 2. 创建两个食物管理器
    backgroundFood1 = new FoodManager(1, 2);
    backgroundFood2 = new FoodManager(1, 2);

    // 3. 创建渲染器（不创建新窗口）
    backgroundRenderer = new Renderer();
    backgroundRenderer->Init(1920, 1080, L"", false);

    // 4. 创建P1背景蛇（左侧，蓝色）
    Point startPos1 = {10, 18};                                           // 左侧区域中心（0-21中间）
    backgroundSnake1 = new Snake(0, startPos1, RIGHT, RGB(63, 114, 175)); // #3F72AF
    backgroundAI1 = new AIController(backgroundFood1);
    backgroundSnake1->SetController(backgroundAI1);
    backgroundFood1->SpawnFood(*backgroundSnake1, *backgroundMap1);

    // 5. 创建P2背景蛇（右侧，粉色）
    Point startPos2 = {10, 18};                                           // 地图坐标10，实际坂52（42+10）
    backgroundSnake2 = new Snake(1, startPos2, LEFT, RGB(224, 133, 133)); // #E08585
    backgroundAI2 = new AIController(backgroundFood2);
    backgroundSnake2->SetController(backgroundAI2);
    backgroundFood2->SpawnFood(*backgroundSnake2, *backgroundMap2);

    // 6. 初始化计数器
    backgroundUpdateCounter = 0;
}

void MenuScene::UpdateBackgroundSnake()
{
    // 每20帧更新一次（约0.2秒，比游戏速度慢）
    backgroundUpdateCounter++;
    if (backgroundUpdateCounter >= 20)
    {
        backgroundUpdateCounter = 0;

        // ===== 更新P1（左侧蛇） =====
        if (backgroundSnake1 && backgroundSnake1->IsAlive())
        {
            backgroundSnake1->Update(*backgroundMap1);
            Point head1 = backgroundSnake1->GetHead();

            // 检查是否越界到右侧（超过x=21）
            if (head1.x > 21)
            {
                // 重生
                int newX = rand() % 18 + 2;
                int newY = rand() % 32 + 2;
                backgroundSnake1->Reset(Point(newX, newY), (Direction)(rand() % 4));
                backgroundFood1->ClearAllFoods();
                backgroundFood1->SpawnFood(*backgroundSnake1, *backgroundMap1);
            }
            else
            {
                // 检查吃食物
                if (backgroundFood1->HasFoodAt(head1))
                {
                    backgroundFood1->ConsumeFood(head1);
                    backgroundSnake1->Grow();
                    if (backgroundFood1->NeedMoreFood())
                        backgroundFood1->SpawnFood(*backgroundSnake1, *backgroundMap1);
                    if (backgroundSnake1->GetLength() > 40)
                        backgroundSnake1->ShrinkByHalf();
                }

                // 检查碰撞
                WallType wall1 = backgroundMap1->GetWallType(head1);
                if (wall1 != NO_WALL || backgroundSnake1->CheckSelfCollision())
                {
                    int newX = rand() % 18 + 2;
                    int newY = rand() % 32 + 2;
                    backgroundSnake1->Reset(Point(newX, newY), (Direction)(rand() % 4));
                    backgroundFood1->ClearAllFoods();
                    backgroundFood1->SpawnFood(*backgroundSnake1, *backgroundMap1);
                }
            }
        }

        // ===== 更新P2（右侧蛇） =====
        if (backgroundSnake2 && backgroundSnake2->IsAlive())
        {
            backgroundSnake2->Update(*backgroundMap2);
            Point head2 = backgroundSnake2->GetHead();

            // P2的实际坐标需要转换（地图坐标0对应实际坐标42）
            int actualX = head2.x + 42;

            // 检查是否越界到左侧（小于x=42）
            if (actualX < 42)
            {
                // 重生
                int newX = rand() % 18 + 2;
                int newY = rand() % 32 + 2;
                backgroundSnake2->Reset(Point(newX, newY), (Direction)(rand() % 4));
                backgroundFood2->ClearAllFoods();
                backgroundFood2->SpawnFood(*backgroundSnake2, *backgroundMap2);
            }
            else
            {
                // 检查吃食物
                if (backgroundFood2->HasFoodAt(head2))
                {
                    backgroundFood2->ConsumeFood(head2);
                    backgroundSnake2->Grow();
                    if (backgroundFood2->NeedMoreFood())
                        backgroundFood2->SpawnFood(*backgroundSnake2, *backgroundMap2);
                    if (backgroundSnake2->GetLength() > 40)
                        backgroundSnake2->ShrinkByHalf();
                }

                // 检查碰撞
                WallType wall2 = backgroundMap2->GetWallType(head2);
                if (wall2 != NO_WALL || backgroundSnake2->CheckSelfCollision())
                {
                    int newX = rand() % 18 + 2;
                    int newY = rand() % 32 + 2;
                    backgroundSnake2->Reset(Point(newX, newY), (Direction)(rand() % 4));
                    backgroundFood2->ClearAllFoods();
                    backgroundFood2->SpawnFood(*backgroundSnake2, *backgroundMap2);
                }
            }
        }
    }
}
void MenuScene::RenderBackgroundSnake()
{
    if (!backgroundRenderer)
        return;

    // 绘制P1（左侧蛇）
    if (backgroundSnake1)
    {
        backgroundRenderer->DrawSnake(*backgroundSnake1);
    }

    // 绘制P2（右侧蛇）- 手动绘制并添加坐标偏移
    if (backgroundSnake2)
    {
        const auto &body = backgroundSnake2->GetBody();
        if (!body.empty())
        {
            // 绘制蛇头（带阴影，粉色）
            COLORREF headColor = RGB(224, 133, 133); // #E08585
            int headX = body[0].x + 42;
            int headY = body[0].y;

            // 使用Renderer的私有方法需要公开，或直接用EasyX绘制
            int pixelX = headX * 30; // BLOCK_SIZE = 30
            int pixelY = headY * 30;

            // 绘制阴影
            setfillcolor(RGB(219, 226, 239));
            setlinecolor(RGB(249, 247, 247)); // 背景色边框
            setlinestyle(PS_SOLID, 2);
            fillroundrect(pixelX + 2, pixelY + 2, pixelX + 31, pixelY + 31, 8, 8);
            // 绘制蛇头
            setfillcolor(headColor);
            setlinecolor(RGB(249, 247, 247)); // #F9F7F7 背景色边框，制造间隙感
            setlinestyle(PS_SOLID, 2);
            fillroundrect(pixelX, pixelY, pixelX + 29, pixelY + 29, 8, 8);

            // 绘制蛇身
            for (size_t i = 1; i < body.size(); ++i)
            {
                int bodyX = (body[i].x + 42) * 30;
                int bodyY = body[i].y * 30;
                setfillcolor(headColor);
                setlinecolor(RGB(249, 247, 247)); // #F9F7F7 背景色边框
                setlinestyle(PS_SOLID, 2);
                fillroundrect(bodyX, bodyY, bodyX + 29, bodyY + 29, 8, 8);
            }

            // 绘制P2标签
            wchar_t label[] = L"P2";
            settextstyle(24, 0, L"微软雅黑");
            settextcolor(RGB(17, 45, 78));
            setbkmode(TRANSPARENT);
            int textWidth = textwidth(label);
            outtextxy(pixelX + (30 - textWidth) / 2, pixelY - 28, label);
        }
    }
}

void MenuScene::CleanupBackgroundSnake()
{
    // 按相反顺序清理资源
    if (backgroundSnake1)
    {
        delete backgroundSnake1;
        backgroundSnake1 = nullptr;
    }

    if (backgroundSnake2)
    {
        delete backgroundSnake2;
        backgroundSnake2 = nullptr;
    }

    if (backgroundAI1)
    {
        delete backgroundAI1;
        backgroundAI1 = nullptr;
    }

    if (backgroundAI2)
    {
        delete backgroundAI2;
        backgroundAI2 = nullptr;
    }

    if (backgroundFood1)
    {
        delete backgroundFood1;
        backgroundFood1 = nullptr;
    }

    if (backgroundFood2)
    {
        delete backgroundFood2;
        backgroundFood2 = nullptr;
    }

    if (backgroundRenderer)
    {
        delete backgroundRenderer;
        backgroundRenderer = nullptr;
    }

    if (backgroundMap1)
    {
        delete backgroundMap1;
        backgroundMap1 = nullptr;
    }

    if (backgroundMap2)
    {
        delete backgroundMap2;
        backgroundMap2 = nullptr;
    }
}

// ============== 网络模式处理 ==============

// 任务 7: 处理网络对战模式选择
bool MenuScene::HandleNetworkMode()
{
    // 获取玩家昵称（如果未设置）
    if (playerName.empty() || playerName == L"Player")
    {
        wchar_t nameBuffer[40] = L"玩家1";
        if (!InputBox(nameBuffer, 40, L"请输入您的昵称：", L"联机对战", L"玩家1"))
        {
            // 用户取消
            return false;
        }
        playerName = nameBuffer;
        if (playerName.empty())
        {
            playerName = L"玩家1";
        }
    }

    // 初始化网络管理器（如果还未初始化）- 仅创建对象，不连接
    if (!networkMgr)
    {
        networkMgr = new NetworkManager();
    }

    // 显示联机模式选择场景
    NetworkModeScene modeScene(false);
    NetworkModeAction action = modeScene.Show();

    switch (action)
    {
    case NM_CREATE_ROOM:
        // 创建房间流程（房主）
        return CreateRoomFlow();

    case NM_JOIN_ROOM:
        // 加入房间流程（客机）
        return JoinRoomFlow();

    case NM_BACK:
        // 返回菜单
        return false;

    default:
        return false;
    }
}

// 任务 8: 创建房间流程
bool MenuScene::CreateRoomFlow()
{
    if (!networkMgr)
    {
        MessageBoxW(GetHWnd(), L"网络未初始化！", L"错误", MB_OK | MB_ICONERROR);
        return false;
    }

    // 【房主流程】连接到本地服务器（自动启动serv.exe）
    cleardevice();
    settextcolor(RGB(17, 45, 78));
    settextstyle(48, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);
    const wchar_t *msg = L"正在启动本地服务器...";
    int msgWidth = textwidth(msg);
    outtextxy(960 - msgWidth / 2, 500, msg);
    FlushBatchDraw();

    std::string serverIp = "127.0.0.1";
    int serverPort = 10001;
    std::string playerNameStr = WstringToUtf8(playerName);

    if (!networkMgr->Connect(serverIp, serverPort, playerNameStr))
    {
        MessageBoxW(GetHWnd(),
                    L"启动本地服务器失败！\n\n可能的原因：\n1. serv.exe 文件不存在\n2. 端口 10001 被占用\n3. 防火墙阻止连接",
                    L"连接错误",
                    MB_OK | MB_ICONERROR);
        return false;
    }

    // 输入房间名
    wchar_t roomNameBuffer[40] = L"我的房间";
    if (!InputBox(roomNameBuffer, 40, L"请输入房间名称：", L"创建房间", L"我的房间"))
    {
        // 用户取消
        return false;
    }

    std::wstring roomName = roomNameBuffer;
    if (roomName.empty())
    {
        roomName = L"我的房间";
    } // 创建房间
    uint32_t roomId = 0;
    std::string roomNameStr = WstringToUtf8(roomName);

    if (!networkMgr->CreateRoom(roomNameStr, roomId))
    {
        MessageBoxW(GetHWnd(), L"创建房间失败！", L"错误", MB_OK | MB_ICONERROR);
        return false;
    }

    // 进入大厅循环（房主模式）
    bool isReturningFromGame = false;
    while (true)
    {
        LobbyScene lobby(networkMgr, roomId, roomName, playerName, true, false, isReturningFromGame);
        bool shouldStart = lobby.Show();

        if (!shouldStart)
        {
            // 用户退出房间
            networkMgr->LeaveRoom();
            return false;
        }

        // 双方准备好，开始游戏
        GameManager::StartNetworkGame(networkMgr, true); // 房主模式

        // 游戏结束，返回 LobbyScene 继续循环
        isReturningFromGame = true; // 标记为从游戏返回
    }
}

// 任务 9: 加入房间流程
bool MenuScene::JoinRoomFlow()
{
    if (!networkMgr)
    {
        MessageBoxW(GetHWnd(), L"网络未初始化！", L"错误", MB_OK | MB_ICONERROR);
        return false;
    }

    // 【客机流程】输入房主IP地址
    wchar_t ipBuffer[40] = L"192.168.1.100";
    if (!InputBox(ipBuffer, 40, L"请输入房主的IP地址：\n(例如: 192.168.1.100)", L"加入房间", L"192.168.1.100"))
    {
        // 用户取消
        return false;
    }

    // 将宽字符IP转换为普通字符串
    std::wstring ipWstr = ipBuffer;
    std::string serverIp;
    for (wchar_t wc : ipWstr)
    {
        if (wc < 128) // 只保留ASCII字符
        {
            serverIp += (char)wc;
        }
    }

    if (serverIp.empty())
    {
        MessageBoxW(GetHWnd(), L"IP地址不能为空！", L"错误", MB_OK | MB_ICONERROR);
        return false;
    }

    // 连接到指定的房主IP
    cleardevice();
    settextcolor(RGB(17, 45, 78));
    settextstyle(48, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);
    wchar_t connectMsg[100];
    swprintf_s(connectMsg, L"正在连接 %s ...", ipBuffer);
    int msgWidth = textwidth(connectMsg);
    outtextxy(960 - msgWidth / 2, 500, connectMsg);
    FlushBatchDraw();

    int serverPort = 10001;
    std::string playerNameStr = WstringToUtf8(playerName);

    if (!networkMgr->Connect(serverIp, serverPort, playerNameStr))
    {
        wchar_t errMsg[200];
        swprintf_s(errMsg, L"连接失败！\n\n目标IP: %s\n端口: %d\n\n可能的原因：\n1. 房主未开启房间\n2. IP地址错误\n3. 网络不通\n4. 防火墙阻止", ipBuffer, serverPort);
        MessageBoxW(GetHWnd(), errMsg, L"连接错误", MB_OK | MB_ICONERROR);
        return false;
    }

    // 显示房间列表
    RoomListScene roomList(networkMgr, false);
    bool joinSuccess = roomList.Show();

    if (!joinSuccess)
    {
        // 用户取消或加入失败
        return false;
    }

    uint32_t roomId = roomList.GetJoinedRoomId();

    // 获取房间信息
    std::vector<RoomInfo> rooms;
    networkMgr->GetRoomList(rooms);

    std::wstring roomName = L"房间";
    for (const auto &room : rooms)
    {
        if (room.roomId == roomId)
        {
            int roomNameLen = (int)wcslen(room.roomName);
            roomName = std::wstring(room.roomName, room.roomName + roomNameLen);
            break;
        }
    }

    // 进入大厅循环（客人模式）
    bool isReturningFromGame = false;
    while (true)
    {
        LobbyScene lobby(networkMgr, roomId, roomName, playerName, false, false, isReturningFromGame);
        bool shouldStart = lobby.Show();

        if (!shouldStart)
        {
            // 用户退出房间
            networkMgr->LeaveRoom();
            return false;
        }

        // 双方准备好，开始游戏
        GameManager::StartNetworkGame(networkMgr, false); // 客机模式

        // 游戏结束，返回 LobbyScene 继续循环
        isReturningFromGame = true; // 标记为从游戏返回
    }
}
