#include "GameManager.h"
#include "KeyboardController.h"
#include "NetworkController.h"
#include "AIController.h"
#include <iostream>
#include <fstream>
#include <ctime>

// ============== 构造与析构 ==============
GameManager::GameManager()
    : playerSnake(nullptr), gameMap(nullptr), foodManager(nullptr),
      renderer(nullptr), networkManager(nullptr),
      currentState(MENU), currentMode(SINGLE),
      score(0), highScore(0), lives(3), gameTime(0), wallCollisions(0),
      player1Score(0), player2Score(0), player1Time(0), player2Time(0),
      isPaused(false), isRunning(false),
      recordFileName("game_records.txt")
{
}

GameManager::~GameManager()
{
    Cleanup();
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
    delete networkManager;

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
                Update();
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
    }

    // 2. 更新所有蛇
    for (auto snake : snakes)
    {
        if (snake && snake->IsAlive())
        {
            snake->Update(*gameMap);
        }
    }

    // 3. 更新游戏时间
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

    // 更新最高分
    if (score > highScore)
    {
        highScore = score;
        std::ofstream outFile("highscore.txt");
        if (outFile.is_open())
        {
            outFile << highScore;
            outFile.close();
        }
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

            // 记录双人游戏结束时的得分和时长
            if (snakes.size() >= 2)
            {
                player1Score = snakes[0] ? snakes[0]->GetLength() : 0;
                player2Score = snakes[1] ? snakes[1]->GetLength() : 0;
                player1Time = snakes[0] && snakes[0]->IsAlive() ? gameTime : 0;
                player2Time = snakes[1] && snakes[1]->IsAlive() ? gameTime : 0;
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
    int buttonY = windowHeight / 2 + 150;  // 与Renderer中的位置一致
    int buttonWidth = 400;                 // 与Renderer中的宽度一致
    int buttonHeight = 100;                // 与Renderer中的高度一致

    bool waitingForInput = true;

    while (waitingForInput)
    {
        // 更新输入缓冲
        inputMgr.Update();

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
    renderer->DrawUI(score, highScore, snakeLength, lives, gameTime);

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

    // 检测退出按钮点击（使用缓冲系统，不丢失点击）
    if (renderer)
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
    // 遍历所有蛇，让它们的键盘控制器缓存输入
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
void GameManager::CheckCollisions()
{
    // 遍历所有蛇，检查每条蛇的碰撞
    for (auto snake : snakes)
    {
        if (!snake || !snake->IsAlive())
            continue;

        Point head = snake->GetHead();

        // 1. 检测撞墙
        WallType wallType = gameMap->GetWallType(head);
        if (wallType != NO_WALL)
        {
            if (wallType == HARD_WALL || wallType == BOUNDARY)
            {
                if (snake == playerSnake)
                {
                    // 玩家蛇撞墙，根据模式处理
                    if (currentMode == BEGINNER)
                    {
                        // 入门版：撞墙直接死亡，游戏结束
                        snake->SetAlive(false);
                        lives = 0;
                    }
                    else if (currentMode == ADVANCED)
                    {
                        // 进阶版：蛇挂掉后蛇尸变边界，再生成新蛇
                        HandleAdvancedModeDeath();
                    }
                    else if (currentMode == EXPERT)
                    {
                        // 高级版：蛇尸变食物
                        // TODO: 实现高级版逻辑
                        snake->SetAlive(false);
                        lives = 0;
                    }
                    else
                    {
                        // 其他模式：原有逻辑
                        snake->SetAlive(false);
                        lives = 0;
                    }
                }
                else
                {
                    // AI蛇或其他蛇撞墙，直接死亡
                    HandleSnakeDeath(snake);
                }
            }
            else if (wallType == SOFT_WALL)
            {
                // 软墙：移除墙壁
                gameMap->RemoveWall(head);
            }
        }

        // 2. 检测撞自己
        if (snake->CheckSelfCollision())
        {
            if (snake == playerSnake)
            {
                // 玩家蛇撞自己，根据模式处理
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
                    // 高级版：蛇尸变食物
                    // TODO: 实现高级版逻辑
                    snake->SetAlive(false);
                    lives = 0;
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
            int points = foodManager->ConsumeFood(head);

            // 只有玩家蛇的分数计入总分
            if (snake == playerSnake)
            {
                score += points;
            }

            // 所有蛇都能生长
            snake->Grow();
        }
    }

    // 检查是否需要生成更多食物
    if (foodManager->NeedMoreFood() && !snakes.empty())
    {
        // 使用第一条蛇作为参考来生成食物（避免生成在蛇身上）
        foodManager->SpawnFood(*snakes[0], *gameMap);
    }
}

// ============== 记录管理 ==============
void GameManager::SaveGameRecord()
{
    std::ofstream file(recordFileName, std::ios::app);
    if (file.is_open())
    {
        // 获取当前时间
        time_t now = time(nullptr);
        char timeStr[100];
        tm timeinfo;
        localtime_s(&timeinfo, &now);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

        // 保存记录
        file << "时间:" << timeStr
             << " 模式:" << GetModeString(currentMode)
             << " 得分:" << score
             << " 长度:" << (playerSnake ? playerSnake->GetLength() : 0)
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

    // 生成初始食物
    foodManager->SpawnFood(*playerSnake, *gameMap);

    // 初始化游戏状态
    score = 0;
    lives = 3;
    gameTime = 0;
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
    snake2->SetController(new AIController(foodManager)); // AI控制，传入食物管理器
    snakes.push_back(snake2);

    // 生成初始食物
    foodManager->SpawnFood(*playerSnake, *gameMap);

    // 初始化游戏状态
    score = 0;
    lives = 3;
    gameTime = 0;
    player1Score = 0;
    player2Score = 0;
    player1Time = 0;
    player2Time = 0;
}

void GameManager::InitNetworkPVPMode()
{
    // 网络对战模式（暂时未实现，使用本地双人逻辑）
    InitLocalPVPMode();
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
    std::ifstream file("highscore.txt");
    if (file.is_open())
    {
        file >> highScore;
        file.close();
    }
    else
    {
        highScore = 0;
    }

    // 保存最高分
    if (score > highScore)
    {
        std::ofstream outFile("highscore.txt");
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

    // 多人游戏模式：检查是否只剩一条蛇活着
    if (currentMode == LOCAL_PVP || currentMode == NET_PVP || currentMode == PVE)
    {
        int aliveCount = 0;
        for (auto snake : snakes)
        {
            if (snake && snake->IsAlive())
            {
                aliveCount++;
            }
        }

        // 如果只剩一条蛇或所有蛇都死了，游戏结束
        if (aliveCount <= 1)
        {
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
