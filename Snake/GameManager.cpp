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
    bool isNewHighScore = (score == highScore && score > 0);
    if (renderer)
    {
        // 绘制游戏结束界面
        renderer->BeginBatch();
        renderer->DrawGameOverScreen(score, isNewHighScore);
        renderer->EndBatch();
    }

    // 等待用户交互
    Sleep(500); // 短暂延迟

    // 定义按钮区域
    int buttonX = 300;
    int buttonY = 400;
    int buttonWidth = 200;
    int buttonHeight = 50;

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
    renderer->Clear();
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
    if (!playerSnake || !playerSnake->IsAlive())
        return;

    Point head = playerSnake->GetHead();

    // 1. 检测撞墙
    WallType wallType = gameMap->GetWallType(head);
    if (wallType != NO_WALL)
    {
        if (wallType == HARD_WALL || wallType == BOUNDARY)
        {
            // 单人模式：撞墙直接死亡
            if (currentMode == SINGLE || currentMode == BEGINNER)
            {
                playerSnake->SetAlive(false);
                lives = 0; // 直接游戏结束
            }
            else
            {
                // 多人/进阶模式：蛇长度减半
                playerSnake->ShrinkByHalf();
                wallCollisions++;

                if (playerSnake->GetLength() < 2)
                {
                    playerSnake->SetAlive(false);
                    lives--;
                }
            }
        }
        else if (wallType == SOFT_WALL)
        {
            // 软墙：移除墙壁
            gameMap->RemoveWall(head);
        }
    }

    // 2. 检测撞自己
    if (playerSnake->CheckSelfCollision())
    {
        playerSnake->SetAlive(false);
        lives = 0; // 撞自己直接结束
    }

    // 3. 检测撞其他蛇（多蛇模式）
    if (snakes.size() > 1)
    {
        for (size_t i = 1; i < snakes.size(); ++i)
        {
            if (snakes[i] && snakes[i]->IsAlive())
            {
                if (playerSnake->CheckCollisionWith(*snakes[i]))
                {
                    HandleSnakeDeath(playerSnake);
                }
            }
        }
    }
}

void GameManager::HandleFood()
{
    if (!playerSnake || !foodManager)
        return;

    Point head = playerSnake->GetHead();

    if (foodManager->HasFoodAt(head))
    {
        int points = foodManager->ConsumeFood(head);
        score += points;
        playerSnake->Grow();

        if (foodManager->NeedMoreFood())
        {
            foodManager->SpawnFood(*playerSnake, *gameMap);
        }
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
    int totalWidth = MAP_WIDTH * BLOCK_SIZE + 200; // 游戏区域 + UI区域
    renderer->Init(totalWidth, MAP_HEIGHT * BLOCK_SIZE, L"贪吃蛇游戏", false);

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
    int totalWidth = MAP_WIDTH * BLOCK_SIZE + 200;
    renderer->Init(totalWidth, MAP_HEIGHT * BLOCK_SIZE, L"贪吃蛇 - 本地双人", false);

    // 创建地图
    gameMap = new GameMap();

    // 创建两条蛇
    Point startPos1(MAP_WIDTH / 3, MAP_HEIGHT / 2);
    Snake *snake1 = new Snake(0, startPos1, RIGHT, RGB(0, 255, 0));
    snake1->SetController(new KeyboardController(0)); // WASD
    snakes.push_back(snake1);
    playerSnake = snake1;

    Point startPos2(MAP_WIDTH * 2 / 3, MAP_HEIGHT / 2);
    Snake *snake2 = new Snake(1, startPos2, LEFT, RGB(255, 255, 0));
    snake2->SetController(new KeyboardController(1)); // 方向键
    snakes.push_back(snake2);

    // 创建食物管理器
    foodManager = new FoodManager();
    foodManager->SpawnFood(*playerSnake, *gameMap);
}

void GameManager::InitNetworkPVPMode()
{
    // 网络对战模式（暂不实现）
    InitSingleMode();
}

void GameManager::InitPVEMode()
{
    // 人机对战模式（暂不实现）
    InitSingleMode();
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
    // 检查玩家蛇是否死亡
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
