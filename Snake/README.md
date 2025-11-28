# 贪吃蛇游戏 - 项目结构说明

## 📂 项目结构

本项目采用 **MVC（Model-View-Controller）变体模式**，确保逻辑与显示分离，控制与本体分离。

### 1. 基础公共模块 (Infrastructure)

这些文件被几乎所有其他文件包含，定义了通用的数据类型。

- **Common.h** - 全局常量、枚举、结构体定义

  - Point 结构体（坐标）
  - Direction 枚举（方向）
  - GameMode 枚举（游戏模式）
  - FoodType 枚举（食物类型）
  - GameState 枚举（游戏状态）
  - WallType 枚举（墙壁类型）

- **Utils.h** - 工具函数集合
  - 随机数生成
  - 碰撞检测辅助函数
  - 距离计算函数

### 2. 接口层 (Interfaces) —— 解耦的核心

这是"进可攻退可守"的关键。所有具体逻辑都依赖这些接口，而不是具体的类。

- **IController.h** - 控制器接口
  - `Direction MakeDecision(const Snake& snake, const GameMap& map)` - 核心决策方法

### 3. 游戏实体模块 (Game Objects / Model)

这些类只负责游戏逻辑（数据变化），不负责画图，也不负责读取键盘。

- **Snake.h / Snake.cpp** - 蛇的本体

  - 持有 `IController*` 指针（关键解耦设计）
  - `Update(GameMap& map)` - 每帧调用，向控制器询问方向
  - `Move()` / `Grow()` - 移动和生长逻辑
  - 碰撞检测方法

- **GameMap.h / GameMap.cpp** - 地图数据管理

  - 存储墙壁数据
  - `IsWall(Point p)` - 判断是否撞墙
  - `ConvertSnakeToWalls()` - 蛇尸变墙（进阶版）

- **FoodManager.h / FoodManager.cpp** - 食物生成与管理
  - 支持多种食物类型（普通、BUFF、毒药等）
  - `SpawnFood()` - 生成食物（避开蛇身）
  - `ConsumeFood()` - 吃食物获得分数

### 4. 控制策略模块 (Strategies / Controller)

这里实现具体的控制逻辑。你可以分阶段编写。

- **KeyboardController.h / .cpp** - 本地玩家控制器

  - 使用 `GetAsyncKeyState` 检测键盘输入
  - 支持 WASD 和方向键
  - 防止 180 度转向

- **NetworkController.h / .cpp** - 网络（对手）控制器

  - 从 `NetworkManager` 读取对手操作
  - 不检测键盘输入

- **AIController.h / .cpp** - 电脑控制器（加分项）
  - 使用 BFS 或 A\* 算法寻路
  - 安全性评估函数

### 5. 渲染模块 (View)

只负责画画，不负责逻辑。

- **Renderer.h / .cpp** - 封装 EasyX 操作
  - `Init()` - 初始化图形窗口
  - `DrawSnake()` / `DrawMap()` / `DrawFood()` - 绘制游戏元素
  - `DrawUI()` - 绘制 UI 信息
  - 批量绘图优化

### 6. 网络通信模块 (Network)

把助教的代码隔离在这里，不要让它污染其他文件。

- **Client.h** - 助教提供的网络框架

  - **不要修改这个文件**（除非修 Bug）

- **NetworkManager.h / .cpp** - 游戏逻辑与底层网络的适配器
  - `GamePacket` 结构体 - 定义通信协议
  - `Connect()` - 连接服务器
  - `SendMyInput()` / `GetOpponentInput()` - 发送/接收操作
  - `Update()` - 每帧驱动底层接收

### 7. 系统核心模块 (System)

大管家，负责把上面所有东西串起来。

- **GameManager.h / .cpp** - 核心状态机

  - 管理所有游戏对象（蛇、地图、食物、渲染器等）
  - `Init(GameMode mode)` - 根据模式初始化不同的控制器
  - `Run()` - 主循环
  - 游戏记录系统（保存/加载/查询）

- **MenuScene.h / .cpp** - 菜单场景

  - 绘制菜单按钮
  - 检测鼠标/键盘输入
  - 返回用户选择的 `GameMode`

- **main.cpp** - 程序入口
  - 越简单越好，只负责创建菜单和游戏管理器

## 🚀 开发路线建议

### Step 1: 基础搭建（已完成）

✅ 创建所有 .h 和 .cpp 文件  
✅ 把类名和方法名写好  
⏳ 确保编译通过（下一步）

### Step 2: 可视化

- 实现 `Renderer` 的基本绘图功能
- 实现 `GameManager` 的主循环
- 目标：画出一条不动的蛇

### Step 3: 动起来

- 实现 `Snake::Move()` 和 `Snake::Update()`
- 实现 `KeyboardController::MakeDecision()`
- 目标：能用键盘控制蛇移动

### Step 4: 完善单机

- 实现 `FoodManager`（生成和检测食物）
- 实现 `GameMap`（撞墙判定）
- 实现 `GameManager` 的完整游戏循环
- 目标：能玩完整的单机版

### Step 5: 网络接入（关键点）

- 引入助教的 `Client.h`
- 实现 `NetworkManager`
- 实现 `NetworkController`
- 在 `GameManager` 里根据模式切换控制器
- 目标：能进行网络对战

### Step 6: 高级功能

- 实现 `AIController`（人机对战）
- 实现进阶版/高级版规则
- 实现游戏记录系统
- 实现 `MenuScene`

## 📝 关键设计思想

### 1. 控制器模式（核心）

```cpp
// Snake 持有接口指针，不关心具体实现
class Snake {
    IController* controller;  // 可以是键盘/网络/AI

    void Update(GameMap& map) {
        // 向控制器询问方向
        Direction dir = controller->MakeDecision(*this, map);
        // 根据方向移动
    }
};
```

### 2. 模式切换（灵活性）

```cpp
// GameManager 根据模式创建不同的控制器
void GameManager::Init(GameMode mode) {
    if (mode == NET_PVP) {
        p1->SetController(new KeyboardController());
        p2->SetController(new NetworkController(networkMgr));
    } else if (mode == PVE) {
        p1->SetController(new KeyboardController());
        p2->SetController(new AIController());
    }
}
```

### 3. 分层架构（可维护性）

- **Model**（Snake/GameMap/FoodManager）：只管数据和逻辑
- **View**（Renderer）：只管绘制
- **Controller**（各种 Controller）：只管决策
- **System**（GameManager）：把它们组合起来

## ⚠️ 注意事项

1. **所有 `TODO` 标记的地方都需要你实现**
2. 保持接口不变，只实现函数体
3. 测试时逐步添加功能，不要一次全写完
4. 网络部分可以最后实现，先确保单机版能玩
5. 记得处理内存释放，避免内存泄漏
6. 不要修改助教提供的 `Client.h`（当你获得真实代码后）

## 📚 编译配置

### Visual Studio 项目设置

1. 创建一个新的 C++ 空项目
2. 将所有 `.cpp` 文件添加到项目
3. 配置 EasyX 库：
   - 下载并安装 EasyX
   - 项目属性 → C/C++ → 常规 → 附加包含目录 → 添加 EasyX 头文件路径
   - 项目属性 → 链接器 → 输入 → 附加依赖项 → 添加 `EasyXw.lib`（或 `EasyXa.lib`）

### 需要的库

- EasyX（图形库）
- Windows SDK（键盘和网络）
- 标准 C++ 库

## 🎮 游戏功能清单

### 基础项（必须实现）

- [x] 入门版：撞墙/撞身游戏结束
- [x] 进阶版：蛇尸变边界
- [x] 高级版：蛇尸变食物
- [x] 历史记录功能（文件保存）
- [x] 菜单功能
- [x] 实时 UI（蛇长/生命/分数/最高分/时间）

### 加分项（可选实现）

- [x] 人机对战（AI 控制器）
- [x] 本地双人对战
- [x] 网络对战
- [ ] 软墙壁/硬墙壁
- [ ] 加速区/减速区
- [ ] 体能槽系统
- [ ] 精灵果/恶果
- [ ] 游戏存档功能
- [ ] 大地图+镜头跟随
- [ ] RPG 模式

## 🔧 调试技巧

1. **先测试静态显示**：在 `Renderer` 里先画一条固定的蛇
2. **再测试移动**：让蛇自动向右移动，不用键盘
3. **然后加键盘控制**：实现 `KeyboardController`
4. **最后加复杂功能**：食物、碰撞、网络等

## 📞 获取帮助

如果遇到问题：

1. 检查编译错误，确保所有头文件正确包含
2. 使用调试器单步跟踪，查看变量值
3. 先实现简单版本，再逐步添加功能
4. 网络部分可以咨询助教

祝你开发顺利！🐍
