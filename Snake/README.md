# 贪吃蛇游戏 - 项目文档

## 📋 项目概述

这是一个基于 **EasyX 图形库** 开发的多模式贪吃蛇游戏，支持单人模式、本地双人对战、网络对战和人机对战。项目采用 **MVC 架构** + **策略模式**，实现了逻辑与显示分离、控制策略可插拔的设计。

### 核心特性

- ✅ **7 种游戏模式**：入门版/进阶版/高级版/单人/本地双人/网络对战/人机对战
- ✅ **智能 AI 系统**：基于 BFS 寻路算法的电脑对手
- ✅ **完整 UI 系统**：主菜单 + 游戏场景 + 历史记录查看
- ✅ **游戏记录系统**：自动保存对局历史，支持筛选和分页查看
- ✅ **网络对战支持**：基于 TCP 协议的实时对战
- ✅ **多种食物类型**：普通食物、加分食物、精灵果、恶果等

---

## 📂 项目结构

### 架构图解

```
┌─────────────────────────────────────────────────────────┐
│                      main.cpp                           │
│                   (程序入口)                             │
└───────────────────┬─────────────────────────────────────┘
                    │
        ┌───────────▼────────────┐
        │   MenuScene.cpp/h      │  ← 主菜单场景
        │   HistoryScene.cpp/h   │  ← 历史记录场景
        └───────────┬────────────┘
                    │
        ┌───────────▼────────────┐
        │  GameManager.cpp/h     │  ← 游戏核心管理器
        │  (状态机 + 主循环)      │
        └───────┬────────────────┘
                │
    ┌───────────┼───────────────┐
    │           │               │
┌───▼───┐   ┌──▼──┐      ┌────▼─────┐
│ Model │   │View │      │Controller│
└───┬───┘   └──┬──┘      └────┬─────┘
    │          │              │
    │    ┌─────▼────┐    ┌────▼──────────────┐
    │    │Renderer  │    │ IController (接口) │
    │    └──────────┘    └────┬──────────────┘
    │                          │
    ├─ Snake.cpp/h         ┌───┴──────────────────┐
    ├─ GameMap.cpp/h       │                      │
    ├─ FoodManager.cpp/h   ├─ KeyboardController  │
    │                      ├─ NetworkController   │
    └─ Common.h            ├─ AIController        │
       (数据定义)           └──────────────────────┘
```

### 文件说明

#### 1. 核心系统层

| 文件                  | 说明                                                                           |
| --------------------- | ------------------------------------------------------------------------------ |
| **main.cpp**          | 程序入口，负责显示菜单并启动游戏                                               |
| **GameManager.h/cpp** | 游戏核心管理器，实现主循环、状态机、记录系统                                   |
| **Common.h**          | 全局数据定义（Point、Direction、GameMode、GameState、FoodType 等枚举和结构体） |

#### 2. 场景层 (UI Scenes)

| 文件                   | 说明                                            |
| ---------------------- | ----------------------------------------------- |
| **MenuScene.h/cpp**    | 主菜单场景，包含 7 个模式选择按钮和历史记录入口 |
| **HistoryScene.h/cpp** | 历史记录查看场景，支持模式筛选、分页浏览        |
| **InputManager.h/cpp** | 输入管理器，封装键盘和鼠标输入检测              |

#### 3. 模型层 (Game Logic)

| 文件                  | 说明                                                      |
| --------------------- | --------------------------------------------------------- |
| **Snake.h/cpp**       | 蛇实体类，持有 IController 指针，负责移动、生长、碰撞检测 |
| **GameMap.h/cpp**     | 地图管理类，存储墙壁数据、边界检测、蛇尸转化逻辑          |
| **FoodManager.h/cpp** | 食物管理器，负责生成多种类型食物、碰撞检测、分数计算      |

#### 4. 视图层 (Rendering)

| 文件               | 说明                                                |
| ------------------ | --------------------------------------------------- |
| **Renderer.h/cpp** | 渲染器，封装 EasyX 绘图操作，绘制蛇、地图、食物、UI |

#### 5. 控制层 (Controller Strategies)

| 文件                         | 说明                                          |
| ---------------------------- | --------------------------------------------- |
| **IController.h**            | 控制器接口（纯虚类），定义决策方法            |
| **KeyboardController.h/cpp** | 键盘控制器，检测 WASD/方向键，防止 180 度转向 |
| **NetworkController.h/cpp**  | 网络控制器，从 NetworkManager 读取对手操作    |
| **AIController.h/cpp**       | AI 控制器，基于 BFS 算法实现寻路和安全性评估  |

#### 6. 网络层

| 文件                     | 说明                                   |
| ------------------------ | -------------------------------------- |
| **Client.h**             | TCP 网络通信底层封装（第三方提供）     |
| **NetworkManager.h/cpp** | 网络管理器，适配游戏逻辑与底层网络接口 |

#### 7. 数据文件

| 文件                 | 说明                                          |
| -------------------- | --------------------------------------------- |
| **game_records.txt** | 游戏历史记录（CSV 格式：时间,模式,分数,长度） |

---

## 🎮 游戏模式详解

### 基础难度模式

| 模式                  | 说明         | 规则                         |
| --------------------- | ------------ | ---------------------------- |
| **入门版 (BEGINNER)** | 单人经典模式 | 撞墙/撞自己游戏结束          |
| **进阶版 (ADVANCED)** | 蛇尸变边界   | 死亡后蛇身转化为墙壁障碍     |
| **高级版 (EXPERT)**   | 蛇尸变食物   | 死亡后蛇身转化为食物可被吃掉 |

### 对战模式

| 模式                     | 说明           | 特点                   |
| ------------------------ | -------------- | ---------------------- |
| **单人模式 (SINGLE)**    | 无限制单人游戏 | 纯粹的休闲模式         |
| **本地双人 (LOCAL_PVP)** | 同屏对战       | P1: WASD，P2: 方向键   |
| **网络对战 (NET_PVP)**   | TCP 联机对战   | 需连接服务器，实时同步 |
| **人机对战 (PVE)**       | 对抗 AI        | AI 使用 BFS 寻路算法   |

---

## 🏗️ 核心设计模式

### 1. 策略模式 (Strategy Pattern)

通过 `IController` 接口实现控制策略的可插拔设计：

```cpp
// Snake 持有接口指针，运行时动态切换策略
class Snake {
    IController* controller;  // 可以是键盘/网络/AI

    void Update(GameMap& map) {
        Direction dir = controller->MakeDecision(*this, map);
        // 根据决策移动
    }
};
```

**优势**：

- 新增控制方式无需修改 Snake 类
- 可在运行时动态切换控制器
- 易于测试和调试

### 2. MVC 架构 (Model-View-Controller)

```
Model (数据层)          View (视图层)         Controller (控制层)
    Snake  ─────────────► Renderer ◄────────── KeyboardController
    GameMap                   │                NetworkController
    FoodManager               │                AIController
                              │
                        GameManager
                       (协调者/状态机)
```

**职责分离**：

- **Model**：只负责游戏逻辑和数据
- **View**：只负责渲染显示
- **Controller**：只负责接收输入和决策

### 3. 状态机模式 (State Machine)

GameManager 通过状态机管理游戏流程：

```
MENU ──► PLAYING ──► GAME_OVER ──► MENU
          ▲    │
          └────┘ (PAUSED)
```

### 4. 工厂模式思想

根据 GameMode 动态创建不同的控制器组合：

```cpp
void GameManager::InitByGameMode(GameMode mode) {
    switch(mode) {
        case LOCAL_PVP:
            player1->SetController(new KeyboardController(VK_W, VK_S, VK_A, VK_D));
            player2->SetController(new KeyboardController(VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT));
            break;
        case NET_PVP:
            player1->SetController(new KeyboardController());
            player2->SetController(new NetworkController(networkMgr));
            break;
        case PVE:
            player1->SetController(new KeyboardController());
            player2->SetController(new AIController());
            break;
    }
}
```

---

## 🛠️ 技术实现要点

### 1. 碰撞检测系统

```cpp
// 蛇与食物的碰撞
Point head = snake->GetHead();
if (foodManager->CheckCollision(head)) {
    int score = foodManager->ConsumeFood(head);
    snake->Grow();
}

// 蛇与墙壁的碰撞
if (gameMap->IsWall(head) || snake->CheckSelfCollision()) {
    GameOver();
}
```

### 2. AI 寻路算法

AIController 使用 **BFS (广度优先搜索)** 寻找到食物的最短路径：

1. 从蛇头开始 BFS 搜索
2. 遍历四个方向（上下左右）
3. 避开墙壁和蛇身
4. 找到食物后回溯路径
5. 返回第一步的方向

**安全性检查**：如果找不到安全路径，尝试向空间最大的方向移动。

### 3. 历史记录系统

**数据格式 (CSV)**：

```
2025-12-07 13:13:22,LOCAL_PVP,30,5
时间,模式,分数,蛇长
```

**功能**：

- 每局游戏结束自动保存
- 支持按模式筛选（全部/单人/双人）
- 分页显示（每页 5 条）
- 显示历史最高分

### 4. 网络同步机制

使用 TCP 协议进行实时数据同步：

```cpp
// 发送本地操作
GamePacket packet;
packet.direction = localSnake->GetDirection();
networkMgr->Send(packet);

// 接收对手操作
GamePacket opponentPacket = networkMgr->Receive();
networkController->SetDirection(opponentPacket.direction);
```

---

## 📚 编译与运行

### 环境要求

- **操作系统**：Windows 10/11
- **编译器**：Visual Studio 2019/2022 (支持 C++17)
- **图形库**：EasyX (20220116 或更高版本)
- **分辨率**：建议 1920×1080

### Visual Studio 配置

1. 安装 EasyX 图形库：

   - 下载地址：https://easyx.cn
   - 运行安装程序，自动配置 VS 环境

2. 打开项目：

   - 使用 `VSC++/Snake/Snake.sln` 或
   - 手动创建项目并添加所有 `.cpp` 文件

3. 编译运行：
   - 配置为 **Release x86** 或 **Debug x86**
   - 按 F5 运行

### 注意事项

- EasyX 仅支持 Windows 平台
- 需要管理员权限读写 `game_records.txt`
- 网络模式需要连接到服务器（IP 和端口在代码中配置）

---

## ✨ 已实现功能

### 核心游戏系统 ✅

- ✅ 完整的 MVC 架构设计
- ✅ 7 种游戏模式支持
- ✅ 碰撞检测系统（墙壁、自身、食物）
- ✅ 多种食物类型（普通、加分、精灵果、恶果）
- ✅ 蛇的生长和移动逻辑
- ✅ 游戏状态机管理

### 控制系统 ✅

- ✅ 键盘控制器（支持 WASD 和方向键）
- ✅ 网络控制器（TCP 实时同步）
- ✅ AI 控制器（BFS 寻路算法）
- ✅ 防止 180 度转向逻辑

### UI 系统 ✅

- ✅ 主菜单场景（7 个模式按钮）
- ✅ 游戏场景（实时显示分数、蛇长、时间）
- ✅ 历史记录场景（筛选、分页、最高分卡片）
- ✅ 游戏结束界面
- ✅ 胶囊式筛选按钮
- ✅ 响应式布局

### 数据系统 ✅

- ✅ 游戏记录自动保存（CSV 格式）
- ✅ 历史记录查看（支持按模式筛选）
- ✅ 分页浏览功能
- ✅ 最高分统计

### 网络系统 ✅

- ✅ TCP 客户端封装
- ✅ 游戏数据包协议
- ✅ 实时对战同步
- ✅ 网络错误处理

---

## 🎯 代码亮点

### 1. 接口设计

通过 `IController` 纯虚接口实现策略模式，所有控制器继承同一接口：

```cpp
class IController {
public:
    virtual Direction MakeDecision(const Snake& snake, const GameMap& map) = 0;
    virtual ~IController() = default;
};
```

### 2. 智能指针管理

使用智能指针避免内存泄漏：

```cpp
std::unique_ptr<Snake> player1;
std::unique_ptr<Snake> player2;
std::unique_ptr<FoodManager> foodManager;
```

### 3. CSV 数据持久化

使用简洁的 CSV 格式存储游戏记录：

```cpp
void GameManager::SaveGameRecord() {
    std::ofstream file("game_records.txt", std::ios::app);
    file << GetCurrentTime() << ","
         << GetModeString(currentMode) << ","
         << score << ","
         << player1->GetLength() << "\n";
}
```

### 4. BFS 寻路算法

AI 使用广度优先搜索找到食物的最短路径：

```cpp
Direction AIController::BFS_FindPath(Point start, Point target, const GameMap& map) {
    std::queue<Point> q;
    std::map<Point, Point> parent;
    q.push(start);

    while (!q.empty()) {
        Point current = q.front();
        q.pop();

        if (current == target) {
            // 回溯找到第一步方向
            return TraceBackFirstStep(start, target, parent);
        }

        // 遍历四个方向...
    }
}
```

---

## 📊 项目统计

- **总代码行数**：约 3500+ 行
- **类的数量**：15 个
- **头文件**：15 个
- **实现文件**：13 个
- **支持模式**：7 种
- **开发周期**：约 2 周

---

## 📖 参考资料

- [EasyX 官方文档](https://docs.easyx.cn/)
- [C++ MVC 架构设计](https://www.geeksforgeeks.org/mvc-design-pattern/)
- [BFS 算法详解](https://www.geeksforgeeks.org/breadth-first-search-or-bfs-for-a-graph/)
- [策略模式](https://refactoring.guru/design-patterns/strategy)

---

## 📝 更新日志

### v1.0 (2025-12-07)

- ✅ 完成所有核心功能
- ✅ 实现 7 种游戏模式
- ✅ 添加历史记录系统
- ✅ 优化 UI 界面（胶囊式筛选、分页功能）
- ✅ 修复列对齐问题
- ✅ 添加翻页功能

---

## 👨‍💻 作者

课程大作业项目 - 高级程序设计

**开发环境**：Visual Studio 2022 + EasyX + Windows 11

---

🎮 **Enjoy the Game!**
