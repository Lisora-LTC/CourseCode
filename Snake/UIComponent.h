#pragma once
#include <graphics.h>
#include <string>
#include <functional>

// ============== UI组件基类 ==============
// 所有UI组件的抽象基类，提供通用的位置、尺寸、交互接口
class UIComponent
{
protected:
    int x, y;          // 组件位置
    int width, height; // 组件尺寸
    bool visible;      // 是否可见
    bool enabled;      // 是否启用（禁用时不响应交互）

public:
    UIComponent(int x = 0, int y = 0, int width = 0, int height = 0)
        : x(x), y(y), width(width), height(height), visible(true), enabled(true) {}

    virtual ~UIComponent() {}

    // ============== 渲染方法 ==============
    /**
     * @brief 绘制组件（纯虚函数，由子类实现）
     */
    virtual void Draw() = 0;

    // ============== 交互方法 ==============
    /**
     * @brief 检测鼠标是否在组件内
     */
    virtual bool IsMouseOver(int mouseX, int mouseY) const
    {
        if (!visible || !enabled)
            return false;
        return mouseX >= x && mouseX <= x + width &&
               mouseY >= y && mouseY <= y + height;
    }

    /**
     * @brief 处理鼠标点击事件
     * @return 是否消费了该事件
     */
    virtual bool OnClick(int mouseX, int mouseY)
    {
        return IsMouseOver(mouseX, mouseY);
    }

    // ============== 访问器 ==============
    void SetPosition(int newX, int newY)
    {
        x = newX;
        y = newY;
    }
    void SetSize(int newWidth, int newHeight)
    {
        width = newWidth;
        height = newHeight;
    }
    void SetVisible(bool isVisible) { visible = isVisible; }
    void SetEnabled(bool isEnabled) { enabled = isEnabled; }

    int GetX() const { return x; }
    int GetY() const { return y; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    bool IsVisible() const { return visible; }
    bool IsEnabled() const { return enabled; }
};

// ============== 按钮样式枚举 ==============
enum ButtonStyle
{
    BUTTON_CAPSULE,      // 胶囊形（两端圆形）
    BUTTON_ROUNDED_RECT, // 圆角矩形
    BUTTON_RECT          // 直角矩形
};

// ============== 按钮状态枚举 ==============
enum ButtonState
{
    NORMAL,  // 正常状态
    HOVER,   // 鼠标悬停
    PRESSED, // 按下状态
    DISABLED // 禁用状态
};

// ============== 文本对齐枚举 ==============
enum TextAlign
{
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT
};

// ============== 通用按钮类 ==============
class UIButton : public UIComponent
{
private:
    std::wstring text; // 按钮文字
    ButtonStyle style; // 按钮样式
    ButtonState state; // 当前状态

    // 颜色配置（不同状态）
    COLORREF normalColor;
    COLORREF hoverColor;
    COLORREF pressedColor;
    COLORREF disabledColor;
    COLORREF textColor;
    COLORREF hoverTextColor;

    // 样式配置
    int cornerRadius; // 圆角半径
    int fontSize;     // 字体大小
    bool hasShadow;   // 是否有阴影
    int shadowOffset; // 阴影偏移量

    // 回调函数
    std::function<void()> onClick; // 点击回调

public:
    // ============== 构造函数 ==============
    UIButton(int x, int y, int width, int height,
             const std::wstring &text = L"",
             ButtonStyle style = BUTTON_CAPSULE)
        : UIComponent(x, y, width, height),
          text(text), style(style), state(NORMAL),
          normalColor(RGB(219, 226, 239)),    // #DBE2EF 默认灰蓝
          hoverColor(RGB(85, 132, 188)),      // #5584BC 悬停亮蓝
          pressedColor(RGB(63, 114, 175)),    // #3F72AF 按下深蓝
          disabledColor(RGB(200, 200, 200)),  // 禁用灰色
          textColor(RGB(17, 45, 78)),         // #112D4E 深藏青
          hoverTextColor(RGB(255, 255, 255)), // 白色
          cornerRadius(10), fontSize(48), hasShadow(false), shadowOffset(5),
          onClick(nullptr)
    {
    }

    // ============== 绘制方法 ==============
    void Draw() override;

    // ============== 交互方法 ==============
    /**
     * @brief 更新按钮状态（根据鼠标位置）
     */
    void UpdateState(int mouseX, int mouseY, bool isPressed = false);

    /**
     * @brief 处理点击事件
     */
    bool OnClick(int mouseX, int mouseY) override;

    // ============== 配置方法 ==============
    void SetText(const std::wstring &newText) { text = newText; }
    void SetStyle(ButtonStyle newStyle) { style = newStyle; }
    void SetColors(COLORREF normal, COLORREF hover, COLORREF pressed = RGB(0, 0, 0));
    void SetTextColor(COLORREF normal, COLORREF hover = RGB(255, 255, 255));
    void SetFontSize(int size) { fontSize = size; }
    void SetCornerRadius(int radius) { cornerRadius = radius; }
    void SetShadow(bool enabled, int offset = 5)
    {
        hasShadow = enabled;
        shadowOffset = offset;
    }
    void SetOnClick(std::function<void()> callback) { onClick = callback; }

    // ============== 访问器 ==============
    ButtonState GetState() const { return state; }
    std::wstring GetText() const { return text; }

private:
    /**
     * @brief 绘制胶囊形按钮
     */
    void DrawCapsule(COLORREF bgColor, COLORREF txtColor);

    /**
     * @brief 绘制圆角矩形按钮
     */
    void DrawRoundedRect(COLORREF bgColor, COLORREF txtColor);

    /**
     * @brief 绘制直角矩形按钮
     */
    void DrawRect(COLORREF bgColor, COLORREF txtColor);

    /**
     * @brief 绘制按钮文字（居中）
     */
    void DrawText(COLORREF txtColor);
};

// ============== 卡片组件类 ==============
class UICard : public UIComponent
{
private:
    COLORREF backgroundColor;
    COLORREF borderColor;
    int cornerRadius;
    bool hasShadow;
    int shadowOffset;

public:
    UICard(int x, int y, int width, int height)
        : UIComponent(x, y, width, height),
          backgroundColor(RGB(255, 255, 255)),
          borderColor(RGB(219, 226, 239)),
          cornerRadius(15),
          hasShadow(true),
          shadowOffset(5) {}

    void Draw() override;

    // 配置方法
    void SetBackgroundColor(COLORREF color) { backgroundColor = color; }
    void SetBorderColor(COLORREF color) { borderColor = color; }
    void SetCornerRadius(int radius) { cornerRadius = radius; }
    void SetShadow(bool enabled, int offset = 5)
    {
        hasShadow = enabled;
        shadowOffset = offset;
    }
};

// ============== 状态指示点枚举 ==============
enum DotStatus
{
    DOT_GREEN, // 绿色（等待中/在线）
    DOT_RED,   // 红色（游戏中/离线）
    DOT_GRAY,  // 灰色（满员/禁用）
    DOT_BLUE,  // 蓝色（准备就绪）
    DOT_YELLOW // 黄色（警告）
};

// ============== 状态指示点组件 ==============
class UIStatusDot : public UIComponent
{
private:
    int radius;
    DotStatus status;

public:
    UIStatusDot(int x, int y, int radius = 8, DotStatus status = DOT_GRAY);

    void Draw() override;

    // 设置状态
    void SetStatus(DotStatus newStatus) { status = newStatus; }
    DotStatus GetStatus() const { return status; }
};

// ============== 玩家卡片组件 ==============
class UIPlayerCard : public UIComponent
{
private:
    bool isWaiting;           // 是否等待中
    std::wstring playerName;  // 玩家昵称
    std::wstring statusText;  // 状态文字
    COLORREF avatarColor;     // 头像颜色
    COLORREF borderColor;     // 边框颜色
    COLORREF backgroundColor; // 背景颜色
    int cornerRadius;         // 圆角半径
    int avatarSize;           // 头像大小

public:
    UIPlayerCard(int x, int y, int width, int height);

    void Draw() override;

    // 设置为等待状态
    void SetWaiting(bool waiting) { isWaiting = waiting; }

    // 设置玩家信息
    void SetPlayerInfo(const std::wstring &name, const std::wstring &status, COLORREF color)
    {
        playerName = name;
        statusText = status;
        avatarColor = color;
        isWaiting = false;
    }

    // 设置颜色
    void SetColors(COLORREF avatar, COLORREF border, COLORREF background)
    {
        avatarColor = avatar;
        borderColor = border;
        backgroundColor = background;
    }

    bool IsWaiting() const { return isWaiting; }

private:
    void DrawWaitingState();
    void DrawPlayerInfo();
};

// ============== 加载动画类型 ==============
enum AnimationType
{
    ANIM_BREATHING_CIRCLE, // 呼吸圆（圆环半径周期变化）
    ANIM_RADAR_SCAN,       // 雷达扫描（扇形旋转）
    ANIM_DOTS_JUMPING      // 三点跳动
};

// ============== 加载动画组件 ==============
class UILoadingAnimation : public UIComponent
{
private:
    int size;           // 动画大小
    AnimationType type; // 动画类型
    int animationFrame; // 当前帧数
    int maxFrames;      // 总帧数（一个周期）
    COLORREF color;     // 动画颜色

public:
    UILoadingAnimation(int x, int y, int size = 50, AnimationType type = ANIM_BREATHING_CIRCLE);

    void Draw() override;

    // 更新动画（每帧调用）
    void Update() { animationFrame = (animationFrame + 1) % maxFrames; }

    // 设置动画类型
    void SetAnimationType(AnimationType newType) { type = newType; }

    // 设置颜色
    void SetColor(COLORREF newColor) { color = newColor; }

private:
    void DrawBreathingCircle(int centerX, int centerY);
    void DrawRadarScan(int centerX, int centerY);
    void DrawDotsJumping(int centerX, int centerY);
};
