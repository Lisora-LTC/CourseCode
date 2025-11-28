#include "GameManager.h"
#include "KeyboardController.h"
#include "NetworkController.h"
#include "AIController.h"
#include <windows.h>
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
    const DWORD updateInterval = GAME_SPEED;

    while (isRunning && currentState != MENU)
    {
        HandleInput();

        DWORD currentTime = GetTickCount();
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

        Render();
        Sleep(10); // 降低CPU占用
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
        foodManager->ClearAllFood();
        foodManager->SpawnFood(*playerSnake, *gameMap);
    }

    currentState = PLAYING;
}

void GameManager::GameOver()
{
    currentState = GAME_OVER;

    // 保存记录
    SaveGameRecord();

    // 显示游戏结束界面
    bool isNewHighScore = (score == highScore && score > 0);
    if (renderer)
    {
        renderer->DrawGameOverScreen(score, isNewHighScore);
    }

    // 等待用户按键
    Sleep(1000);
    while (true)
    {
        if (GetAsyncKeyState(VK_RETURN) & 0x8000)
        {
            break;
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            break;
        }
        Sleep(100);
    }

    isRunning = false;
}
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
    // 检测暂停键（空格）
    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        static bool spacePressed = false;
        if (!spacePressed)
        {
            TogglePause();
            spacePressed = true;
        }
    }
    else
    {
        static bool spacePressed = false;
        spacePressed = false;
    }

    // 检测ESC键（退出）
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
        isRunning = false;
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
            // 硬墙或边界：蛇长度减半
            playerSnake->ShrinkByHalf();
            wallCollisions++;

            if (playerSnake->GetLength() < 2)
            {
                playerSnake->Die();
                lives--;
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
        HandleSnakeDeath(playerSnake);
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
    // ============== 记录管理 ==============
    void GameManager::SaveGameRecord()
    {
        std::ofstream file(recordFileName, std::ios::app);
        if (file.is_open())
        {
            // 获取当前时间
            time_t now = time(nullptr);
            char timeStr[100];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));

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
        // ============== 私有方法 ==============
        void GameManager::InitSingleMode()
        {
            // 1. 创建渲染器
            renderer = new Renderer();
            int totalWidth = MAP_WIDTH * BLOCK_SIZE + 200; // 游戏区域 + UI区域
            renderer->Init(totalWidth, MAP_HEIGHT * BLOCK_SIZE, L"贪吃蛇游戏");

            // 2. 创建地图
            gameMap = new GameMap();

            // 3. 创建蛇并设置键盘控制器
            Point startPos(MAP_WIDTH / 2, MAP_HEIGHT / 2);
            playerSnake = new Snake(startPos, RIGHT, 0);
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
            // 创建渲染器
            renderer = new Renderer();
            int totalWidth = MAP_WIDTH * BLOCK_SIZE + 200;
            renderer->Init(totalWidth, MAP_HEIGHT * BLOCK_SIZE, L"贪吃蛇 - 本地双人");

            // 创建地图
            gameMap = new GameMap();

            // 创建两条蛇
            Point startPos1(MAP_WIDTH / 3, MAP_HEIGHT / 2);
            Snake *snake1 = new Snake(startPos1, RIGHT, 0);
            snake1->SetController(new KeyboardController(0)); // WASD
            snakes.push_back(snake1);
            playerSnake = snake1;

            Point startPos2(MAP_WIDTH * 2 / 3, MAP_HEIGHT / 2);
            Snake *snake2 = new Snake(startPos2, LEFT, 1);
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

        void GameManager::HandleSnakeDeath(Snake * snake)
        {
            if (!snake)
                return;

            snake->Die();

            // 根据模式处理蛇尸
            if (currentMode == ADVANCED)
            {
                // 进阶版：蛇尸变墙
                if (gameMap)
                {
                    gameMap->ConvertSnakeToWalls(*snake);
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
                        food.score = 10;
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
        td::vector<GameRecord> GameManager::QueryRecordsByUser(const std::string &username)
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

        // ============== 私有方法 ==============
        void GameManager::InitSingleMode()
        {
            // TODO: 初始化单人模式
            // 1. 创建地图
            // 2. 创建蛇
            // 3. 设置键盘控制器
            // 4. 创建食物管理器
        }

        void GameManager::InitLocalPVPMode()
        {
            // TODO: 初始化本地双人模式
            // 1. 创建两条蛇
            // 2. 分别设置键盘控制器（P1:WASD, P2:方向键）
        }

        void GameManager::InitNetworkPVPMode()
        {
            // TODO: 初始化网络对战模式
            // 1. 创建网络管理器
            // 2. 连接服务器
            // 3. 创建两条蛇（P1:键盘, P2:网络）
        }

        void GameManager::InitPVEMode()
        {
            // TODO: 初始化人机对战模式
            // 1. 创建两条蛇
            // 2. P1:键盘, P2:AI
        }

        void GameManager::InitByGameMode(GameMode mode)
        {
            // TODO: 根据模式初始化
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
            // TODO: 加载历史最高分
        }

        void GameManager::UpdateGameTime()
        {
            // TODO: 更新游戏时间
        }

        bool GameManager::ShouldGameEnd()
        {
            // TODO: 判断游戏是否应该结束
            // 根据不同模式判断
            return false;
        }

        void GameManager::HandleSnakeDeath(Snake * snake)
        {
            // TODO: 处理蛇死亡
            // 进阶版：蛇尸变墙
            // 高级版：蛇尸变食物
        }
