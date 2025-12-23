#ifndef ENGINE_UI_H
#define ENGINE_UI_H

#include "GlobalConstants.h"
#include "AlertManager.h"
#include <vector>
#include <string>
#include <functional>

/**
 * @struct Point
 * @brief 二维坐标点结构
 */
struct Point
{
    double x;
    double y;

    Point() : x(0.0), y(0.0) {}
    Point(double _x, double _y) : x(_x), y(_y) {}
};

/**
 * @struct Color
 * @brief 颜色结构体（RGBA）
 */
struct Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    Color() : r(255), g(255), b(255), a(255) {}
    Color(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255)
        : r(_r), g(_g), b(_b), a(_a) {}

    // 预定义颜色
    static Color White() { return Color(255, 255, 255); }
    static Color Amber() { return Color(255, 191, 0); } // 琥珀色
    static Color Red() { return Color(255, 0, 0); }
    static Color Black() { return Color(0, 0, 0); }
    static Color Gray() { return Color(128, 128, 128); }
    static Color DarkGray() { return Color(64, 64, 64); }
};

/**
 * @class EngineUI
 * @brief 图形界面类
 *
 * 基于图形库（EasyX/Qt/OpenGL等）进行绘制
 * 处理用户输入和界面刷新
 */
class EngineUI
{
public:
    // ==================== 构造与析构 ====================

    /**
     * @brief 构造函数
     * @param width 窗口宽度
     * @param height 窗口高度
     */
    EngineUI(int width = 1280, int height = 720);

    /**
     * @brief 析构函数
     */
    ~EngineUI();

    // ==================== 初始化与清理 ====================

    /**
     * @brief 初始化图形界面
     * @return true表示初始化成功
     */
    bool initialize();

    /**
     * @brief 关闭图形界面
     */
    void shutdown();

    // ==================== 核心绘制函数 ====================

    /**
     * @brief 更新整个界面
     * @param data 系统数据
     * @param alerts 告警消息列表
     *
     * 调用所有子绘制函数，完成整个界面的渲染
     */
    void update(const SystemData &data, const std::vector<std::string> &alerts);

    /**
     * @brief 清空画面
     */
    void clear();

    /**
     * @brief 刷新显示（将后台缓冲区绘制到屏幕）
     */
    void present();

    // ==================== 表盘绘制函数 ====================

    /**
     * @brief 绘制通用表盘
     * @param value 当前值
     * @param minVal 最小值
     * @param maxVal 最大值
     * @param level 告警级别（决定颜色）
     * @param pos 中心位置
     * @param radius 表盘半径
     * @param label 表盘标签
     *
     * 绘制扇形表盘（0-210度）
     * 根据AlertLevel变换颜色：
     * - NORMAL: 白色
     * - ADVISORY: 白色（有告警文字）
     * - CAUTION: 琥珀色
     * - WARNING: 红色
     * - INVALID: 显示"--"
     */
    void drawGauge(double value, double minVal, double maxVal,
                   AlertLevel level, Point pos, double radius,
                   const std::string &label);

    /**
     * @brief 绘制N1转速表盘
     * @param engine 发动机数据
     * @param level 告警级别
     * @param pos 位置
     */
    void drawN1Gauge(const EngineData &engine, AlertLevel level, Point pos);

    /**
     * @brief 绘制EGT温度表盘
     * @param engine 发动机数据
     * @param level 告警级别
     * @param pos 位置
     */
    void drawEGTGauge(const EngineData &engine, AlertLevel level, Point pos);

    // ==================== 数字显示函数 ====================

    /**
     * @brief 绘制数字指示器
     * @param value 数值
     * @param level 告警级别
     * @param pos 位置
     * @param label 标签
     * @param unit 单位
     * @param precision 小数精度
     */
    void drawDigitalDisplay(double value, AlertLevel level, Point pos,
                            const std::string &label, const std::string &unit,
                            int precision = 1);

    /**
     * @brief 绘制燃油流速显示
     * @param fuelData 燃油数据
     * @param level 告警级别
     * @param pos 位置
     */
    void drawFuelFlowDisplay(const FuelData &fuelData, AlertLevel level, Point pos);

    // ==================== 状态显示函数 ====================

    /**
     * @brief 绘制状态指示器
     * @param data 系统数据
     * @param pos 位置
     *
     * 包括：
     * - START灯（启动阶段亮起）
     * - RUN灯（稳态阶段亮起）
     */
    void drawStatusIndicators(const SystemData &data, Point pos);

    /**
     * @brief 更新指示器状态
     * @param data 系统数据
     *
     * 根据发动机状态更新START/RUN灯的亮暗
     */
    void updateIndicators(const SystemData &data);

    // ==================== 告警显示函数 ====================

    /**
     * @brief 绘制CAS告警消息
     * @param messages 告警消息列表
     * @param pos 起始位置
     *
     * 在界面右侧区域顺序绘制英文告警文字
     * 根据告警级别使用不同颜色
     */
    void drawCASMessages(const std::vector<std::string> &messages, Point pos);

    // ==================== 按钮绘制与交互 ====================

    /**
     * @brief 绘制按钮
     * @param id 按钮ID
     * @param pos 位置
     * @param width 宽度
     * @param height 高度
     * @param label 按钮文字
     * @param enabled 是否可用
     */
    void drawButton(ButtonID id, Point pos, double width, double height,
                    const std::string &label, bool enabled = true);

    /**
     * @brief 绘制所有按钮
     *
     * 包括：
     * - START按钮
     * - STOP按钮
     * - 增加推力按钮
     * - 减小推力按钮
     */
    void drawAllButtons();

    /**
     * @brief 检查鼠标点击
     * @param x 鼠标X坐标
     * @param y 鼠标Y坐标
     * @return 点击的按钮ID，如果未点击任何按钮则返回空
     *
     * 用于处理鼠标点击事件
     */
    ButtonID *checkButtonClick(int x, int y);

    /**
     * @brief 设置按钮回调函数
     * @param callback 回调函数
     *
     * 当按钮被点击时调用
     */
    void setButtonCallback(std::function<void(ButtonID)> callback);

    /**
     * @brief 按钮点击处理
     * @param id 按钮ID
     *
     * 触发回调函数
     */
    void onButtonClicked(ButtonID id);

    // ==================== 事件处理 ====================

    /**
     * @brief 处理输入事件
     * @return true表示继续运行，false表示退出
     *
     * 处理鼠标点击、键盘输入等
     */
    bool processEvents();

    /**
     * @brief 检查窗口是否应该关闭
     * @return true表示应该关闭
     */
    bool shouldClose() const;

private:
    // ==================== 私有成员变量 ====================

    int windowWidth_;  // 窗口宽度
    int windowHeight_; // 窗口高度
    bool initialized_; // 是否已初始化
    bool shouldClose_; // 是否应该关闭

    std::function<void(ButtonID)> buttonCallback_; // 按钮回调函数

    // 状态指示器状态
    bool startLightOn_; // START灯是否亮起
    bool runLightOn_;   // RUN灯是否亮起

    // 按钮位置信息（用于点击检测）
    struct ButtonInfo
    {
        ButtonID id;
        Point pos;
        double width;
        double height;
    };
    std::vector<ButtonInfo> buttons_;

    // ==================== 私有辅助函数 ====================

    /**
     * @brief 根据告警级别获取颜色
     * @param level 告警级别
     * @return 对应的颜色
     */
    Color getColorForLevel(AlertLevel level) const;

    /**
     * @brief 绘制扇形
     * @param center 圆心
     * @param radius 半径
     * @param startAngle 起始角度（度）
     * @param endAngle 结束角度（度）
     * @param color 颜色
     */
    void drawArc(Point center, double radius, double startAngle,
                 double endAngle, Color color);

    /**
     * @brief 绘制文字
     * @param text 文本内容
     * @param pos 位置
     * @param color 颜色
     * @param fontSize 字体大小
     */
    void drawText(const std::string &text, Point pos, Color color, int fontSize = 16);

    /**
     * @brief 绘制矩形
     * @param pos 左上角位置
     * @param width 宽度
     * @param height 高度
     * @param color 颜色
     * @param filled 是否填充
     */
    void drawRect(Point pos, double width, double height, Color color, bool filled = false);

    /**
     * @brief 绘制圆形
     * @param center 圆心
     * @param radius 半径
     * @param color 颜色
     * @param filled 是否填充
     */
    void drawCircle(Point center, double radius, Color color, bool filled = false);

    /**
     * @brief 绘制直线
     * @param start 起点
     * @param end 终点
     * @param color 颜色
     * @param thickness 线宽
     */
    void drawLine(Point start, Point end, Color color, double thickness = 1.0);

    /**
     * @brief 将数值映射到角度
     * @param value 当前值
     * @param minVal 最小值
     * @param maxVal 最大值
     * @return 对应的角度（0-210度）
     */
    double mapValueToAngle(double value, double minVal, double maxVal) const;

    /**
     * @brief 检查点是否在矩形内
     * @param px 点X坐标
     * @param py 点Y坐标
     * @param rect 矩形位置
     * @param width 矩形宽度
     * @param height 矩形高度
     * @return true表示在矩形内
     */
    bool pointInRect(int px, int py, Point rect, double width, double height) const;
};

#endif // ENGINE_UI_H
