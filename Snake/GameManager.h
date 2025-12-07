#pragma once
#include "Common.h"
#include "InputManager.h"
#include "NetworkManager.h"
#include "Snake.h"
#include "GameMap.h"
#include "FoodManager.h"
#include "Renderer.h"
#include <vector>
#include <string>
#include <fstream>

// ============== 游戏记录结构 ==============
struct GameRecord
{
    std::string version;  // 游戏版本（入门版/进阶版/高级版）
    std::string username; // 用户名
    int score;            // 得分
    int length;           // 蛇长度
    int time;             // 游戏时间（秒）

    GameRecord() : score(0), length(0), time(0) {}
};

// ============== 游戏管理器（核心状态机） ==============
class GameManager
{
private:
    // 游戏对象
    std::vector<Snake *> snakes; // 所有蛇（支持多蛇）
    Snake *playerSnake;          // 玩家的蛇
    GameMap *gameMap;
    FoodManager *foodManager;
    Renderer *renderer;
    NetworkManager *networkManager; // 网络管理器（可选）
    InputManager inputMgr;          // 统一输入管理器

    // 游戏状态
    GameState currentState;
    GameMode currentMode;

    // 游戏数据
    int score;
    int highScore;
    int lives;
    int gameTime;       // 游戏时间（秒）
    int wallCollisions; // 撞墙次数（高级版用）

    // 多人游戏数据
    int player1Score; // P1得分
    int player2Score; // P2得分
    int player1Time;  // P1游戏时长
    int player2Time;  // P2游戏时长

    // 控制标志
    bool isPaused;
    bool isRunning;

    // 历史记录
    std::vector<GameRecord> gameRecords;
    std::string recordFileName;

public:
    // ============== 构造与析构 ==============
    GameManager();
    ~GameManager();

    // ============== 初始化 ==============
    /**
     * @brief 初始化游戏（根据模式）
     * @param mode 游戏模式
     */
    void Init(GameMode mode);

    /**
     * @brief 清理游戏资源
     */
    void Cleanup();

    // ============== 主循环 ==============
    /**
     * @brief 运行游戏主循环
     */
    void Run();

    // ============== 游戏逻辑 ==============
    /**
     * @brief 更新游戏状态（每帧调用）
     */
    void Update();

    /**
     * @brief 渲染游戏画面
     */
    void Render();

    /**
     * @brief 处理输入
     */
    void HandleInput();

    /**
     * @brief 高频采样游戏输入（缓存方向键）
     */
    void CacheGameInput();

    /**
     * @brief 检测碰撞
     */
    void CheckCollisions();

    /**
     * @brief 处理食物
     */
    void HandleFood();

    // ============== 游戏控制 ==============
    /**
     * @brief 暂停/继续游戏
     */
    void TogglePause();

    /**
     * @brief 重新开始游戏
     */
    void Restart();

    /**
     * @brief 游戏结束
     */
    void GameOver();

    // ============== 记录管理 ==============
    /**
     * @brief 保存游戏记录
     */
    void SaveGameRecord();

    /**
     * @brief 加载游戏记录
     */
    void LoadGameRecords();

    /**
     * @brief 查询记录（按用户名）
     */
    std::vector<GameRecord> QueryRecordsByUser(const std::string &username);

    /**
     * @brief 显示所有记录
     */
    void DisplayAllRecords();

private:
    /**
     * @brief 初始化单人模式
     */
    void InitSingleMode();

    /**
     * @brief 初始化本地双人模式
     */
    void InitLocalPVPMode();

    /**
     * @brief 初始化网络对战模式
     */
    void InitNetworkPVPMode();

    /**
     * @brief 初始化人机对战模式
     */
    void InitPVEMode();

    /**
     * @brief 根据模式字符串初始化
     */
    void InitByGameMode(GameMode mode);

    /**
     * @brief 加载历史最高分
     */
    void LoadHighScore();

    /**
     * @brief 更新游戏时间
     */
    void UpdateGameTime();

    /**
     * @brief 检查游戏是否应该结束
     */
    bool ShouldGameEnd();

    /**
     * @brief 处理蛇死亡（进阶版/高级版）
     */
    void HandleSnakeDeath(Snake *snake);

    /**
     * @brief 处理进阶版模式的蛇死亡（蛇尸变边界）
     */
    void HandleAdvancedModeDeath();

    /**
     * @brief 检查是否有足够空间生成新蛇和食物
     */
    bool HasEnoughSpace();

    /**
     * @brief 寻找一个空位置
     */
    Point FindEmptyPosition();

    /**
     * @brief 获取模式字符串
     */
    std::string GetModeString(GameMode mode);
};
