#include "GameManager.h"
#include "KeyboardController.h"
#include "NetworkController.h"
#include "AIController.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <windows.h>

// ============== 辅助函数 ==============
// 获取exe所在目录的完整路径
static std::string GetExeDirectory()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);

    // 找到最后一个反斜杠的位置
    size_t pos = exePath.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        return exePath.substr(0, pos + 1);
    }
    return "";
}

// 获取exe目录下的文件完整路径
static std::string GetExeFilePath(const std::string &filename)
{
    return GetExeDirectory() + filename;
}

// ============== 构造与析构 ==============
GameManager::GameManager()
    : playerSnake(nullptr), gameMap(nullptr), foodManager(nullptr),
      renderer(nullptr), networkManager(nullptr), isHost(false),
      ownsNetworkManager(true),
      currentState(MENU), currentMode(SINGLE),
      score(0), highScore(0), lives(3), gameTime(0), wallCollisions(0),
      player1Score(0), player2Score(0), player1Time(0), player2Time(0),
      isPaused(false), isRunning(false),
      recordFileName(GetExeFilePath("game_records.txt"))
{
}

GameManager::~GameManager()
{
    Cleanup();
}

// ============== 静态启动方法 ==============
void GameManager::StartNetworkGame(NetworkManager *networkMgr, bool isHost)
{
    // 创建游戏管理器实例
    GameManager *game = new GameManager();

    // 设置网络管理器（不拥有所有权，由 MenuScene 管理）
    game->networkManager = networkMgr;
    game->ownsNetworkManager = false; // ✅ 不拥有所有权
    game->isHost = isHost;

    // 初始化网络对战模式
    game->Init(NET_PVP);

    // 运行游戏
    game->Run();

    // 清理（游戏结束后，但不会删除 networkManager）
    delete game;
}

// ============== 初始化 ==============
void GameManager::Init(GameMode mode)
{
    currentMode = mode;
    InitByGameMode(mode);
    LoadHighScore();
    LoadGameRecords();
}

void GameManager::Cleanup()
{
    for (auto snake : snakes)
    {
        delete snake;
    }
    snakes.clear();
    playerSnake = nullptr; // 重要：清空悬空指针

    delete gameMap;
    delete foodManager;
    delete renderer;

    // ✅ 只在拥有所有权时才删除 networkManager
    if (ownsNetworkManager)
    {
        delete networkManager;
    }

    gameMap = nullptr;
    foodManager = nullptr;
    renderer = nullptr;
    networkManager = nullptr;
}

// ============== 主循环 ==============
void GameManager::Run()
{
    if (!renderer)
        return;

    isRunning = true;
    currentState = PLAYING;

    // 多人模式显示倒计时（叠加在游戏界面上）
    if (currentMode == LOCAL_PVP || currentMode == NET_PVP || currentMode == PVE)
    {
        // 先渲染一次游戏界面作为背景
        Render();

        for (int i = 3; i > 0; i--)
        {
            BeginBatchDraw();

            // 重新渲染游戏界面
            Render();

            // 显示倒计时数字（白色大字，带阴影效果）
            // 先绘制阴影
            settextcolor(RGB(50, 50, 50));
            settextstyle(120, 0, L"微软雅黑");
            wchar_t countdownText[10];
            swprintf_s(countdownText, L"%d", i);

            int textWidth = textwidth(countdownText);
            int textHeight = textheight(countdownText);
            int x = (getwidth() - textWidth) / 2;
            int y = (getheight() - textHeight) / 2;
            outtextxy(x + 3, y + 3, countdownText); // 阴影

            // 再绘制主文字
            settextcolor(WHITE);
            outtextxy(x, y, countdownText);

            EndBatchDraw();
            Sleep(1000);
        }

        // 显示"开始!"提示
        BeginBatchDraw();
        Render();

        // 先绘制阴影
        settextcolor(RGB(0, 100, 0));
        settextstyle(100, 0, L"微软雅黑");
        wchar_t startText[] = L"开始!";
        int textWidth = textwidth(startText);
        int textHeight = textheight(startText);
        int x = (getwidth() - textWidth) / 2;
        int y = (getheight() - textHeight) / 2;
        outtextxy(x + 3, y + 3, startText); // 阴影

        // 再绘制主文字
        settextcolor(GREEN);
        outtextxy(x, y, startText);
        EndBatchDraw();
        Sleep(500);
    }

    DWORD lastUpdateTime = GetTickCount();
    DWORD lastRenderTime = GetTickCount();
    const DWORD updateInterval = GAME_SPEED; // 游戏逻辑更新：100ms (10 FPS)
    const DWORD renderInterval = 16;         // 渲染更新：16ms (60 FPS)

    while (isRunning && currentState != MENU)
    {
        // 检测窗口是否被关闭
        if (inputMgr.IsWindowClosed())
        {
            exit(0);
        }

        DWORD currentTime = GetTickCount();

        // 更新输入系统（读取所有输入到缓冲区）
        inputMgr.Update();

        // UI输入检测 - 高频率响应（每帧都检测）
        HandleInput();

        // 高频采样游戏输入（缓存方向键输入）
        CacheGameInput(); // 游戏逻辑更新 - 低频率（控制蛇的移动速度）
        if (currentTime - lastUpdateTime >= updateInterval)
        {
            if (!isPaused && currentState == PLAYING)
            {
                // 先检测碰撞，如果死亡就不移动
                CheckCollisionsBeforeMove();
                // 只有存活的蛇才移动
                Update();
                // 移动后再次检测（处理蛇身碰撞等）
                CheckCollisions();
                HandleFood();

                if (ShouldGameEnd())
                {
                    GameOver();
                }
            }
            lastUpdateTime = currentTime;
        }

        // 渲染更新 - 中频率（流畅的视觉反馈）
        if (currentTime - lastRenderTime >= renderInterval)
        {
            Render();
            lastRenderTime = currentTime;
        }

        Sleep(1); // 极小延迟，保持高响应性
    }
}

// ============== 游戏逻辑 ==============
void GameManager::Update()
{
    // 1. 更新网络管理器（如果有）
    if (networkManager)
    {
        networkManager->Update();
        // ✅ 处理房间消息（包括对手输入）
        networkManager->ProcessRoomMessages();
    }

    // 2. 更新所有蛇
    for (auto snake : snakes)
    {
        if (snake && snake->IsAlive())
        {
            snake->Update(*gameMap);
        }
    }

    // 3. 网络模式：发送本地蛇的状态
    if (currentMode == NET_PVP && networkManager && playerSnake && playerSnake->IsAlive())
    {
        // ✅ 发送本地蛇的方向和位置（每次更新后）
        SendPlayerState();
    }

    // 4. 更新游戏时间
    UpdateGameTime();
}

// ============== 游戏控制 ==============
void GameManager::TogglePause()
{
    isPaused = !isPaused;
}

void GameManager::Restart()
{
    // 重置游戏状态
    score = 0;
    lives = 3;
    gameTime = 0;
    wallCollisions = 0;
    isPaused = false;

    // 重置蛇
    if (playerSnake)
    {
        playerSnake->Reset(Point(MAP_WIDTH / 2, MAP_HEIGHT / 2), RIGHT);
    }

    // 清空地图
    if (gameMap)
    {
        gameMap->ClearWalls();
        gameMap->InitBoundaryWalls();
    }

    // 重新生成食物
    if (foodManager && playerSnake && gameMap)
    {
        foodManager->ClearAllFoods();
        foodManager->SpawnFood(*playerSnake, *gameMap);
    }

    currentState = PLAYING;
}

void GameManager::GameOver()
{
    currentState = GAME_OVER;

    // 保存记录
    SaveGameRecord();

    // 更新最高分（比较当前分数和历史最高分）
    if (score > highScore)
    {
        highScore = score;
        // 写入文件保存最高分（使用exe目录下的文件）
        std::string highscoreFile = GetExeFilePath("highscore.txt");
        std::ofstream outFile(highscoreFile);
        if (outFile.is_open())
        {
            outFile << highScore;
            outFile.close();
        }
    }
    else
    {
        // 当前分数没有超过历史最高分，不更新文件
        // highScore保持为加载的历史值
    }

    // 显示游戏结束界面并等待用户选择
    if (renderer)
    {
        renderer->BeginBatch();

        // 判断是否为多人游戏模式
        if (currentMode == LOCAL_PVP || currentMode == NET_PVP || currentMode == PVE)
        {
            // 多人游戏：判断玩家是否胜利，显示双人得分和时长
            bool playerWon = (playerSnake && playerSnake->IsAlive());

            // 网络模式：检查是否收到对手的游戏结束消息
            if (currentMode == NET_PVP && networkManager)
            {
                NetworkManager::ReceivedLobbyState lobbyState;
                if (networkManager->GetReceivedLobbyState(lobbyState) && lobbyState.gameOver)
                {
                    // 根据对手的消息判断胜负（对手赢了我就输了）
                    playerWon = !lobbyState.opponentWon;
                }
            }

            // 记录双人游戏结束时的得分和时长
            if (snakes.size() >= 2)
            {
                player1Score = snakes[0] ? snakes[0]->GetLength() : 0;
                player2Score = snakes[1] ? snakes[1]->GetLength() : 0;

                // 网络模式：根据 isHost 决定显示逻辑
                if (currentMode == NET_PVP)
                {
                    if (isHost)
                    {
                        // 房主显示：P1是我，P2是对手
                        player1Time = gameTime; // 我的时长就是游戏时长
                        player2Time = gameTime; // 对手的时长（近似）
                    }
                    else
                    {
                        // 客机显示：P1是对手，P2是我
                        player1Time = gameTime; // 对手的时长（近似）
                        player2Time = gameTime; // 我的时长就是游戏时长
                    }
                }
                else
                {
                    // 本地模式：根据存活时间
                    player1Time = snakes[0] && snakes[0]->IsAlive() ? gameTime : 0;
                    player2Time = snakes[1] && snakes[1]->IsAlive() ? gameTime : 0;
                }
            }

            renderer->DrawMultiplayerGameOverScreen(playerWon, player1Score, player2Score, player1Time, player2Time);
        }
        else
        {
            // 单人游戏：显示常规结束画面
            bool isNewHighScore = (score == highScore && score > 0);
            renderer->DrawGameOverScreen(score, isNewHighScore);
        }

        renderer->EndBatch();
    }

    // 等待用户交互
    Sleep(500); // 短暂延迟

    // 定义按钮区域（与绘制的按钮位置保持一致）
    int windowWidth = 1920;                // 实际窗口宽度（1080p）
    int windowHeight = 1080;               // 实际窗口高度（1080p）
    int buttonX = (windowWidth - 400) / 2; // 居中
    // 根据游戏模式设置按钮Y坐标（与Renderer中的位置一致）
    int buttonY;
    if (currentMode == LOCAL_PVP || currentMode == NET_PVP || currentMode == PVE)
    {
        buttonY = windowHeight / 2 + 216; // 多人游戏
    }
    else
    {
        buttonY = windowHeight / 2 + 136; // 单人游戏
    }
    int buttonWidth = 400;  // 与Renderer中的宽度一致
    int buttonHeight = 100; // 与Renderer中的高度一致

    bool waitingForInput = true;

    while (waitingForInput)
    {
        // 更新输入缓冲
        inputMgr.Update();

        // ✅ 网络模式：持续更新网络管理器，清理消息队列
        if (currentMode == NET_PVP && networkManager)
        {
            networkManager->Update();
        }

        // 检测鼠标点击（使用缓冲系统）
        MOUSEMSG msg;
        while (inputMgr.GetNextMouseMessage(msg))
        {
            if (msg.uMsg == WM_LBUTTONDOWN)
            {
                // 检测是否点击“返回菜单”按钮
                if (msg.x >= buttonX && msg.x <= buttonX + buttonWidth &&
                    msg.y >= buttonY && msg.y <= buttonY + buttonHeight)
                {
                    waitingForInput = false;
                    break;
                }
            }
        }

        // 检测键盘（使用InputManager自动防抖）
        if (inputMgr.IsKeyJustPressed(VK_RETURN) || inputMgr.IsKeyJustPressed(VK_ESCAPE))
        {
            waitingForInput = false;
        }

        // 检测窗口关闭
        if (inputMgr.IsWindowClosed())
        {
            exit(0);
        }

        Sleep(10);
    }

    isRunning = false;
}

void GameManager::Render()
{
    if (!renderer)
        return;

    renderer->BeginBatch();
    renderer->Clear(RGB(247, 251, 252)); // 游戏背景设置为纯白色
    if (gameMap)
        renderer->DrawMap(*gameMap);
    if (foodManager)
        renderer->DrawFoods(*foodManager);

    // 绘制所有蛇
    renderer->DrawSnakes(snakes);

    // 绘制UI
    int snakeLength = playerSnake ? playerSnake->GetLength() : 0;

    // 实时更新最高分（如果当前分数超过历史最高）
    if (score > highScore)
    {
        highScore = score;
    }

    renderer->DrawUI(score, highScore, snakeLength, lives, gameTime, wallCollisions, currentMode);

    // 如果暂停，显示暂停界面
    if (isPaused)
    {
        renderer->DrawPauseScreen();
    }

    renderer->EndBatch();
}

void GameManager::HandleInput()
{
    // 检测暂停键（空格）- 使用InputManager自动防抖
    if (inputMgr.IsKeyJustPressed(VK_SPACE))
    {
        TogglePause();
    }

    // 检测ESC键（退出）- 使用InputManager自动防抖
    if (inputMgr.IsKeyJustPressed(VK_ESCAPE))
    {
        isRunning = false;
    }

    // 检测退出按钮点击（仅单人模式，使用缓冲系统，不丢失点击）
    bool isMultiplayerMode = (currentMode == LOCAL_PVP || currentMode == NET_PVP || currentMode == PVE);
    if (!isMultiplayerMode && renderer)
    {
        int btnX, btnY, btnWidth, btnHeight;
        renderer->GetExitButtonBounds(btnX, btnY, btnWidth, btnHeight);
        if (inputMgr.IsMouseClickInRect(btnX, btnY, btnWidth, btnHeight, WM_LBUTTONDOWN))
        {
            isRunning = false;
        }
    }
}

void GameManager::CacheGameInput()
{
    // ✅ 网络模式：只处理本地玩家的蛇
    if (currentMode == NET_PVP)
    {
        if (playerSnake && playerSnake->IsAlive())
        {
            IController *controller = playerSnake->GetController();
            if (controller && std::string(controller->GetTypeName()) == "KeyboardController")
            {
                KeyboardController *kbCtrl = dynamic_cast<KeyboardController *>(controller);
                if (kbCtrl)
                {
                    // ✅ 只缓存输入，发送将在蛇实际使用后进行
                    kbCtrl->CacheInput();
                }
            }
        }
        return;
    }

    // ✅ 本地模式：遍历所有蛇
    for (auto snake : snakes)
    {
        if (snake && snake->IsAlive())
        {
            IController *controller = snake->GetController();
            if (controller)
            {
                // 检查是否是键盘控制器
                if (std::string(controller->GetTypeName()) == "KeyboardController")
                {
                    KeyboardController *kbCtrl = dynamic_cast<KeyboardController *>(controller);
                    if (kbCtrl)
                    {
                        kbCtrl->CacheInput();
                    }
                }
            }
        }
    }
}

void GameManager::SendPlayerState()
{
    if (!networkManager || !playerSnake)
        return;

    static Direction lastSentDir = NONE;
    static Point lastSentPos(-1, -1);

    Direction currentDir = playerSnake->GetDirection();
    Point currentPos = playerSnake->GetHead();

    // ✅ 仅在方向或位置改变时发送（避免重复消息）
    if (currentDir != lastSentDir || currentPos != lastSentPos)
    {
        GameCommand cmd;
        cmd.cmdType = CMD_INPUT;
        cmd.data = (int)currentDir; // 将方向转为整数（UP=0, DOWN=1, LEFT=2, RIGHT=3）
        cmd.uid = networkManager->GetMyUID();

        networkManager->SendBinaryMessage((char *)&cmd, sizeof(GameCommand));

        lastSentDir = currentDir;
        lastSentPos = currentPos;
    }
}
// 移动前的碰撞检测（预判下一步是否会撞墙）
void GameManager::CheckCollisionsBeforeMove()
{
    for (auto snake : snakes)
    {
        if (!snake || !snake->IsAlive())
            continue;

        // 预判下一个位置
        Point nextHead = Utils::GetNextPoint(snake->GetHead(), snake->GetDirection());
        WallType wallType = gameMap->GetWallType(nextHead);

        // 只检测撞墙（硬墙和边界）
        if (wallType == HARD_WALL || wallType == BOUNDARY)
        {
            if (snake == playerSnake)
            {
                // 玩家蛇即将撞墙 - 只判定自己的死亡
                if (currentMode == BEGINNER)
                {
                    snake->SetAlive(false);
                    lives = 0;
                }
                else if (currentMode == ADVANCED)
                {
                    HandleAdvancedModeDeath();
                }
                else if (currentMode == EXPERT)
                {
                    wallCollisions++;
                    HandleExpertModeDeath();
                }
                else
                {
                    snake->SetAlive(false);
                    lives = 0;
                }
            }
            else
            {
                HandleSnakeDeath(snake);
            }
        }
    }
}

void GameManager::CheckCollisions()
{
    // 遍历所有蛇，检查每条蛇的碰撞
    for (auto snake : snakes)
    {
        if (!snake || !snake->IsAlive())
            continue;

        Point head = snake->GetHead();

        // 1. 检测撞墙（只处理软墙，硬墙已在CheckCollisionsBeforeMove中处理）
        WallType wallType = gameMap->GetWallType(head);
        if (wallType == SOFT_WALL)
        {
            // 软墙：移除墙壁
            gameMap->RemoveWall(head);
        }

        // 2. 检测撞自己
        if (snake->CheckSelfCollision())
        {
            if (snake == playerSnake)
            {
                // 玩家蛇撞自己，根据模式处理 - 只判定自己的死亡
                if (currentMode == BEGINNER)
                {
                    // 入门版：撞自己直接结束
                    snake->SetAlive(false);
                    lives = 0;
                }
                else if (currentMode == ADVANCED)
                {
                    // 进阶版：蛇挂掉后蛇尸变边界
                    HandleAdvancedModeDeath();
                }
                else if (currentMode == EXPERT)
                {
                    // 高级版：蛇尸变食物，撞墙次数+1
                    wallCollisions++;
                    HandleExpertModeDeath();
                }
                else
                {
                    snake->SetAlive(false);
                    lives = 0;
                }
            }
            else
            {
                // AI蛇或其他蛇撞自己，直接死亡
                HandleSnakeDeath(snake);
            }
        }
    }

    // 3. 检测所有蛇之间的碰撞（多蛇模式）
    if (snakes.size() > 1)
    {
        for (size_t i = 0; i < snakes.size(); ++i)
        {
            if (!snakes[i] || !snakes[i]->IsAlive())
                continue;

            // 检查这条蛇是否撞到其他蛇
            for (size_t j = 0; j < snakes.size(); ++j)
            {
                if (i == j) // 跳过自己
                    continue;

                if (!snakes[j] || !snakes[j]->IsAlive())
                    continue;

                // 检查snakes[i]的头是否撞到snakes[j]的身体
                if (snakes[i]->CheckCollisionWith(*snakes[j]))
                {
                    HandleSnakeDeath(snakes[i]);
                    break; // 这条蛇已经死了，不需要继续检查
                }
            }
        }
    }
}

void GameManager::HandleFood()
{
    if (!foodManager)
        return;

    // 遍历所有蛇，检查是否吃到食物
    for (auto snake : snakes)
    {
        if (!snake || !snake->IsAlive())
            continue;

        Point head = snake->GetHead();

        if (foodManager->HasFoodAt(head))
        {
            // 获取食物信息
            Food *food = foodManager->GetFoodAt(head);
            int points = foodManager->ConsumeFood(head);

            // 只有玩家蛇的分数计入总分
            if (snake == playerSnake)
            {
                score += points;
            }

            // 处理食物效果
            if (food && food->type == SPEED_UP)
            {
                // 加速食物：连续移动两次
                snake->Grow();
                snake->Move();
            }
            else
            {
                // 普通食物：正常生长
                snake->Grow();
            }
        }
    }

    // 检查是否需要生成更多食物
    if (foodManager->NeedMoreFood() && !snakes.empty())
    {
        // 使用第一条蛇作为参考来生成食物（避免生成在蛇身上）
        // 多人模式生成更多食物（1.5倍）
        if (currentMode == LOCAL_PVP || currentMode == NET_PVP || currentMode == PVE)
        {
            // 生成更多食物（1-5个的基础上再额外生成1-3个）
            foodManager->SpawnFood(*snakes[0], *gameMap);
            int extraCount = Utils::RandomInt(1, 3);
            foodManager->SpawnFoodCount(extraCount, *snakes[0], *gameMap);
        }
        else
        {
            foodManager->SpawnFood(*snakes[0], *gameMap);
        }
    }
}

// ============== 记录管理 ==============
void GameManager::SaveGameRecord()
{
    // 使用追加模式打开文件，如果文件不存在会自动创建
    std::ofstream file(recordFileName, std::ios::app);
    if (file.is_open())
    {
        // 获取当前时间
        time_t now = time(nullptr);
        char timeStr[100];
        tm timeinfo;
        localtime_s(&timeinfo, &now);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

        // ✅ 使用CSV格式保存（逗号分隔，使用英文模式名）
        // 格式: 时间,模式,得分,长度
        std::string modeEnglish = GetModeEnglish(currentMode);
        file << timeStr << ","
             << modeEnglish << ","
             << score << ","
             << (playerSnake ? playerSnake->GetLength() : 0)
             << std::endl;

        file.close();
    }
}

void GameManager::LoadGameRecords()
{
    gameRecords.clear();
    // TODO: 从文件加载游戏记录
}

// ============== 私有方法 ==============
void GameManager::InitSingleMode()
{
    // 1. 创建渲染器（不创建窗口，由main管理）
    renderer = new Renderer();
    int totalWidth = 1920;  // 使用1920全屏宽度
    int totalHeight = 1080; // 使用1080全屏高度
    renderer->Init(totalWidth, totalHeight, L"贪吃蛇游戏", false);

    // 2. 创建地图
    gameMap = new GameMap();

    // 3. 创建蛇并设置键盘控制器
    Point startPos(MAP_WIDTH / 2, MAP_HEIGHT / 2);
    playerSnake = new Snake(0, startPos, RIGHT, RGB(0, 255, 0));
    playerSnake->SetController(new KeyboardController(0));
    snakes.push_back(playerSnake);

    // 4. 创建食物管理器
    foodManager = new FoodManager();
    foodManager->SpawnFood(*playerSnake, *gameMap);

    // 5. 初始化游戏状态
    score = 0;
    lives = 3;
    gameTime = 0;
    wallCollisions = 0; // 重置撞墙次数
}

void GameManager::InitLocalPVPMode()
{
    // 创建渲染器（不创建窗口，由main管理）
    renderer = new Renderer();
    int totalWidth = 1920;  // 使用1920全屏宽度
    int totalHeight = 1080; // 使用1080全屏高度
    renderer->Init(totalWidth, totalHeight, L"贪吃蛇 - 本地双人", false);

    // 创建地图
    gameMap = new GameMap();

    // 创建两条蛇 - 背对背方向（都朝外）
    Point startPos1(MAP_WIDTH / 3, MAP_HEIGHT / 2);
    Snake *snake1 = new Snake(0, startPos1, LEFT, RGB(0, 255, 0)); // 左侧蛇朝左（背离中心）
    snake1->SetController(new KeyboardController(0));              // WASD
    snakes.push_back(snake1);
    playerSnake = snake1;

    // 创建食物管理器（先创建，如果有AI需要引用）
    foodManager = new FoodManager();

    Point startPos2(MAP_WIDTH * 2 / 3, MAP_HEIGHT / 2);
    Snake *snake2 = new Snake(1, startPos2, RIGHT, RGB(255, 255, 0)); // 右侧蛇朝右（背离中心）
    snake2->SetController(new KeyboardController(1));                 // 方向键
    snakes.push_back(snake2);

    // 生成初始食物（多人模式1.5倍）
    foodManager->SpawnFood(*playerSnake, *gameMap);
    int extraCount = Utils::RandomInt(1, 3);
    foodManager->SpawnFoodCount(extraCount, *playerSnake, *gameMap);

    // 初始化游戏状态
    score = 0;
    lives = 3;
    gameTime = 0;
    wallCollisions = 0;
    player1Score = 0;
    player2Score = 0;
    player1Time = 0;
    player2Time = 0;
}

void GameManager::InitPVEMode()
{
    // 创建渲染器（不创建窗口，由main管理）
    renderer = new Renderer();
    int totalWidth = 1920;  // 使用1920全屏宽度
    int totalHeight = 1080; // 使用1080全屏高度
    renderer->Init(totalWidth, totalHeight, L"贪吃蛇 - 人机对战", false);

    // 创建地图
    gameMap = new GameMap();

    // 创建玩家蛇（键盘控制）
    Point startPos1(MAP_WIDTH / 3, MAP_HEIGHT / 2);
    Snake *snake1 = new Snake(0, startPos1, LEFT, RGB(0, 255, 0));
    snake1->SetController(new KeyboardController(0)); // WASD
    snakes.push_back(snake1);
    playerSnake = snake1;

    // 创建食物管理器（先创建，AI需要引用）
    foodManager = new FoodManager();

    // 创建AI蛇
    Point startPos2(MAP_WIDTH * 2 / 3, MAP_HEIGHT / 2);
    Snake *snake2 = new Snake(1, startPos2, RIGHT, RGB(255, 0, 255));
    AIController *aiController = new AIController(foodManager); // AI控制，传入食物管理器
    aiController->SetEnemySnake(snake1);                        // 设置敌方蛇为玩家蛇，用于预测和躲避
    snake2->SetController(aiController);
    snakes.push_back(snake2);

    // 生成初始食物（多人模式1.5倍）
    foodManager->SpawnFood(*playerSnake, *gameMap);
    int extraCount = Utils::RandomInt(1, 3);
    foodManager->SpawnFoodCount(extraCount, *playerSnake, *gameMap);

    // 初始化游戏状态
    score = 0;
    lives = 3;
    gameTime = 0;
    wallCollisions = 0;
    player1Score = 0;
    player2Score = 0;
    player1Time = 0;
    player2Time = 0;
}

void GameManager::InitNetworkPVPMode()
{
    // 创建渲染器
    renderer = new Renderer();
    int totalWidth = 1920;
    int totalHeight = 1080;
    renderer->Init(totalWidth, totalHeight, L"贪吃蛇 - 网络对战", false);

    // 创建地图
    gameMap = new GameMap();

    // 创建食物管理器
    foodManager = new FoodManager();

    // 定义两条蛇的初始位置和颜色
    Point p1Pos(MAP_WIDTH / 3, MAP_HEIGHT / 2);     // P1 位置 (左)
    Point p2Pos(MAP_WIDTH * 2 / 3, MAP_HEIGHT / 2); // P2 位置 (右)

    // 创建两个蛇对象
    Snake *snake1 = new Snake(0, p1Pos, LEFT, RGB(0, 255, 0));    // P1 绿色
    Snake *snake2 = new Snake(1, p2Pos, RIGHT, RGB(255, 255, 0)); // P2 黄色

    snakes.push_back(snake1);
    snakes.push_back(snake2);

    // 根据身份分配控制器
    if (isHost)
    {
        // === 我是房主 (P1) ===
        // 我控制绿色蛇 (snake1)
        snake1->SetController(new KeyboardController(0));
        playerSnake = snake1; // 标记我的蛇

        // 对手控制黄色蛇 (snake2)
        snake2->SetController(new NetworkController(networkManager));
    }
    else
    {
        // === 我是客机 (P2) ===
        // 对手控制绿色蛇 (snake1)
        snake1->SetController(new NetworkController(networkManager));

        // 我控制黄色蛇 (snake2)
        snake2->SetController(new KeyboardController(0));
        playerSnake = snake2; // 标记我的蛇
    }

    // 生成初始食物（仅房主生成，后续同步）
    if (isHost)
    {
        foodManager->SpawnFood(*playerSnake, *gameMap);
        int extraCount = Utils::RandomInt(1, 3);
        foodManager->SpawnFoodCount(extraCount, *playerSnake, *gameMap);
    }

    // 初始化游戏状态
    score = 0;
    lives = 3;
    gameTime = 0;
    wallCollisions = 0;
    player1Score = 0;
    player2Score = 0;
    player1Time = 0;
    player2Time = 0;
}

void GameManager::InitByGameMode(GameMode mode)
{
    switch (mode)
    {
    case BEGINNER:
    case ADVANCED:
    case EXPERT:
    case SINGLE:
        InitSingleMode();
        break;
    case LOCAL_PVP:
        InitLocalPVPMode();
        break;
    case NET_PVP:
        InitNetworkPVPMode();
        break;
    case PVE:
        InitPVEMode();
        break;
    }
}

void GameManager::LoadHighScore()
{
    std::string highscoreFile = GetExeFilePath("highscore.txt");

    std::ifstream file(highscoreFile);
    if (file.is_open())
    {
        file >> highScore;
        file.close();
    }
    else
    {
        // 文件不存在，初始化为0
        highScore = 0;
    }

    // 保存最高分（如果当前分数更高）
    if (score > highScore)
    {
        std::ofstream outFile(highscoreFile);
        if (outFile.is_open())
        {
            outFile << highScore;
            outFile.close();
        }
    }
}

void GameManager::UpdateGameTime()
{
    static DWORD startTime = GetTickCount();
    gameTime = (GetTickCount() - startTime) / 1000; // 转换为秒
}

bool GameManager::ShouldGameEnd()
{
    if (currentMode == ADVANCED)
    {
        // 进阶版：检查是否有足够空间
        return !HasEnoughSpace();
    }

    if (currentMode == EXPERT)
    {
        // 高级版：撞墙次数>5 或 空间不足
        return (wallCollisions > 5) || !HasEnoughSpace();
    }

    // 多人游戏模式：检查是否只剩一条蛇活着
    if (currentMode == LOCAL_PVP || currentMode == NET_PVP || currentMode == PVE)
    {
        int aliveCount = 0;
        Snake *winner = nullptr;
        for (auto snake : snakes)
        {
            if (snake && snake->IsAlive())
            {
                aliveCount++;
                winner = snake;
            }
        }

        // 如果只剩一条蛇或所有蛇都死了，游戏结束
        if (aliveCount <= 1)
        {
            // 网络模式：发送游戏结束消息
            if (currentMode == NET_PVP && networkManager)
            {
                GameCommand cmd;
                cmd.cmdType = CMD_GAME_OVER;
                cmd.data = (winner == playerSnake) ? 1 : 0; // 1=本地胜利，0=本地失败
                cmd.uid = networkManager->GetMyUID();
                networkManager->SendBinaryMessage((char *)&cmd, sizeof(GameCommand));
            }
            return true;
        }

        return false;
    }

    // 单人模式：检查玩家蛇是否死亡
    if (playerSnake && !playerSnake->IsAlive())
    {
        return true;
    }

    // 检查生命值
    if (lives <= 0)
    {
        return true;
    }

    return false;
}

void GameManager::HandleAdvancedModeDeath()
{
    if (!playerSnake || !gameMap || !foodManager)
        return;

    wallCollisions++;
    // 3. 检查撞墙次数是否超过5次
    if (wallCollisions > 5)
    {
        playerSnake->SetAlive(false);
        lives = 0;
        GameOver();
        return;
    }

    // 1. 将蛇身变为边界
    const auto &body = playerSnake->GetBody();
    for (const auto &segment : body)
    {
        gameMap->AddWall(segment, HARD_WALL);
    }

    // 2. 检查是否有足够空间生成新蛇
    if (!HasEnoughSpace())
    {
        playerSnake->SetAlive(false);
        lives = 0;
        return;
    }

    // 3. 在随机空位生成新蛇
    Point newPos = FindEmptyPosition();
    Direction newDir = static_cast<Direction>(rand() % 4);
    playerSnake->Reset(newPos, newDir);

    // 4. 重新生成食物
    foodManager->ClearAllFoods();
    foodManager->SpawnFood(*playerSnake, *gameMap);

    // 5. 增加分数（奖励继续游戏）
    score += 10;
}

void GameManager::HandleExpertModeDeath()
{
    if (!playerSnake || !gameMap || !foodManager)
        return;

    // 1. 将蛇身变为食物（不同类型食物）
    const auto &body = playerSnake->GetBody();
    for (size_t i = 0; i < body.size(); ++i)
    {
        const Point &segment = body[i];

        // 蛇头变成加分食物
        if (i == 0)
        {
            foodManager->SpawnFoodAt(segment, BONUS_FOOD);
        }
        // 其他部分变成普通食物
        else
        {
            foodManager->SpawnFoodAt(segment, NORMAL_FOOD);
        }
    }

    wallCollisions++;
    // 2. 检查是否有足够空间生成新蛇（高级版需要更多空间）
    if (!HasEnoughSpace())
    {
        playerSnake->SetAlive(false);
        lives = 0;
        return;
    }

    // 3. 检查撞墙次数是否超过5次
    if (wallCollisions > 5)
    {
        playerSnake->SetAlive(false);
        lives = 0;
        GameOver();
        return;
    }

    // 4. 在随机空位生成新蛇
    Point newPos = FindEmptyPosition();
    Direction newDir = static_cast<Direction>(rand() % 4);
    playerSnake->Reset(newPos, newDir);

    // 5. 生成新的随机食物
    foodManager->SpawnFood(*playerSnake, *gameMap);

    // 6. 增加分数（奖励继续游戏，但比进阶版少）
    score += 5;
}

bool GameManager::HasEnoughSpace()
{
    if (!gameMap)
        return false;

    int emptyCount = 0;
    for (int y = 0; y < MAP_HEIGHT; ++y)
    {
        for (int x = 0; x < MAP_WIDTH; ++x)
        {
            Point p(x, y);
            if (gameMap->GetWallType(p) == NO_WALL)
            {
                emptyCount++;
            }
        }
    }

    // 至少需要 20 个空位（蛇的初始长度 + 食物 + 活动空间）
    return emptyCount >= 20;
}

Point GameManager::FindEmptyPosition()
{
    if (!gameMap)
        return Point(MAP_WIDTH / 2, MAP_HEIGHT / 2);

    // 收集所有空位置
    std::vector<Point> emptyPositions;
    for (int y = 1; y < MAP_HEIGHT - 1; ++y)
    {
        for (int x = 1; x < MAP_WIDTH - 1; ++x)
        {
            Point p(x, y);
            if (gameMap->GetWallType(p) == NO_WALL)
            {
                // 检查是否有蛇占据
                bool occupied = false;
                for (auto snake : snakes)
                {
                    if (snake && snake->IsAlive())
                    {
                        const auto &body = snake->GetBody();
                        for (const auto &seg : body)
                        {
                            if (seg == p)
                            {
                                occupied = true;
                                break;
                            }
                        }
                        if (occupied)
                            break;
                    }
                }

                if (!occupied)
                {
                    emptyPositions.push_back(p);
                }
            }
        }
    }

    if (emptyPositions.empty())
        return Point(MAP_WIDTH / 2, MAP_HEIGHT / 2);

    return emptyPositions[rand() % emptyPositions.size()];
}

void GameManager::HandleSnakeDeath(Snake *snake)
{
    if (!snake)
        return;

    snake->SetAlive(false);

    // 减少生命值
    if (snake == playerSnake)
    {
        lives--;
    }

    // 根据模式处理蛇尸
    if (currentMode == ADVANCED)
    {
        // 进阶版：蛇尸变墙
        if (gameMap)
        {
            gameMap->ConvertSnakeToWalls(snake->GetBody());
        }
    }
    else if (currentMode == EXPERT)
    {
        // 高级版：蛇尸变食物
        if (foodManager && gameMap)
        {
            const auto &body = snake->GetBody();
            for (const auto &point : body)
            {
                // 将蛇身位置生成食物
                Food food;
                food.position = point;
                food.type = NORMAL_FOOD;
                food.scoreValue = 10;
                // 注意：需要在FoodManager中添加直接添加食物的方法
            }
        }
    }

    lives--;
}

std::string GameManager::GetModeString(GameMode mode)
{
    switch (mode)
    {
    case BEGINNER:
        return "入门版";
    case ADVANCED:
        return "进阶版";
    case EXPERT:
        return "高级版";
    case SINGLE:
        return "单人模式";
    case LOCAL_PVP:
        return "本地双人";
    case NET_PVP:
        return "网络对战";
    case PVE:
        return "人机对战";
    default:
        return "未知模式";
    }
}

std::string GameManager::GetModeEnglish(GameMode mode)
{
    switch (mode)
    {
    case BEGINNER:
        return "BEGINNER";
    case ADVANCED:
        return "ADVANCED";
    case EXPERT:
        return "EXPERT";
    case SINGLE:
        return "SINGLE";
    case LOCAL_PVP:
        return "LOCAL_PVP";
    case NET_PVP:
        return "NET_PVP";
    case PVE:
        return "PVE";
    default:
        return "UNKNOWN";
    }
}
std::vector<GameRecord> GameManager::QueryRecordsByUser(const std::string &username)
{
    // TODO: 查询指定用户的记录
    std::vector<GameRecord> results;
    return results;
}

void GameManager::DisplayAllRecords()
{
    // TODO: 显示所有记录
    // 按格式输出到屏幕
}
