#include "UIComponent.h"

// ============== UIButton 实现 ==============

void UIButton::Draw()
{
    if (!visible)
        return;

    // 确定当前颜色
    COLORREF bgColor, txtColor;
    switch (state)
    {
    case HOVER:
        bgColor = hoverColor;
        txtColor = hoverTextColor;
        break;
    case PRESSED:
        bgColor = pressedColor;
        txtColor = hoverTextColor;
        break;
    case DISABLED:
        bgColor = disabledColor;
        txtColor = RGB(150, 150, 150);
        break;
    case NORMAL:
    default:
        bgColor = normalColor;
        txtColor = textColor;
        break;
    }

    // 根据样式绘制
    switch (style)
    {
    case BUTTON_CAPSULE:
        DrawCapsule(bgColor, txtColor);
        break;
    case BUTTON_ROUNDED_RECT:
        DrawRoundedRect(bgColor, txtColor);
        break;
    case BUTTON_RECT:
        DrawRect(bgColor, txtColor);
        break;
    }
}

void UIButton::UpdateState(int mouseX, int mouseY, bool isPressed)
{
    if (!enabled)
    {
        state = DISABLED;
        return;
    }

    bool hover = IsMouseOver(mouseX, mouseY);

    if (isPressed && hover)
    {
        state = PRESSED;
    }
    else if (hover)
    {
        state = HOVER;
    }
    else
    {
        state = NORMAL;
    }
}

bool UIButton::OnClick(int mouseX, int mouseY)
{
    if (!enabled || !visible)
        return false;

    if (IsMouseOver(mouseX, mouseY))
    {
        if (onClick)
        {
            onClick();
        }
        return true;
    }
    return false;
}

void UIButton::SetColors(COLORREF normal, COLORREF hover, COLORREF pressed)
{
    normalColor = normal;
    hoverColor = hover;
    pressedColor = (pressed == RGB(0, 0, 0)) ? hover : pressed;
}

void UIButton::SetTextColor(COLORREF normal, COLORREF hover)
{
    textColor = normal;
    hoverTextColor = hover;
}

// ============== 私有绘制方法 ==============

void UIButton::DrawCapsule(COLORREF bgColor, COLORREF txtColor)
{
    int radius = height / 2;

    // 绘制阴影
    if (hasShadow && state != PRESSED)
    {
        COLORREF shadowColor = RGB(17, 45, 78); // #112D4E
        setfillcolor(shadowColor);
        setlinecolor(shadowColor);
        solidcircle(x + radius + shadowOffset, y + radius + shadowOffset, radius);
        solidcircle(x + width - radius + shadowOffset, y + radius + shadowOffset, radius);
        solidrectangle(x + radius + shadowOffset, y + shadowOffset,
                       x + width - radius + shadowOffset, y + height + shadowOffset);
    }

    // 绘制按钮主体
    setfillcolor(bgColor);
    setlinecolor(bgColor);

    // 左半圆
    solidcircle(x + radius, y + radius, radius);
    // 右半圆
    solidcircle(x + width - radius, y + radius, radius);
    // 中间矩形
    solidrectangle(x + radius, y, x + width - radius, y + height);

    // 绘制文字
    DrawText(txtColor);
}

void UIButton::DrawRoundedRect(COLORREF bgColor, COLORREF txtColor)
{
    // 绘制阴影
    if (hasShadow && state != PRESSED)
    {
        COLORREF shadowColor = RGB(17, 45, 78);
        setfillcolor(shadowColor);
        setlinecolor(shadowColor);
        solidroundrect(x + shadowOffset, y + shadowOffset,
                       x + width + shadowOffset, y + height + shadowOffset,
                       cornerRadius, cornerRadius);
    }

    // 绘制按钮主体
    setfillcolor(bgColor);
    setlinecolor(bgColor);
    solidroundrect(x, y, x + width, y + height, cornerRadius, cornerRadius);

    // 绘制文字
    DrawText(txtColor);
}

void UIButton::DrawRect(COLORREF bgColor, COLORREF txtColor)
{
    // 绘制阴影
    if (hasShadow && state != PRESSED)
    {
        COLORREF shadowColor = RGB(17, 45, 78);
        setfillcolor(shadowColor);
        solidrectangle(x + shadowOffset, y + shadowOffset,
                       x + width + shadowOffset, y + height + shadowOffset);
    }

    // 绘制按钮主体
    setfillcolor(bgColor);
    setlinecolor(bgColor);
    solidrectangle(x, y, x + width, y + height);

    // 绘制文字
    DrawText(txtColor);
}

void UIButton::DrawText(COLORREF txtColor)
{
    if (text.empty())
        return;

    // 设置字体
    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = fontSize;
    f.lfWeight = FW_BOLD;
    wcscpy_s(f.lfFaceName, L"微软雅黑");
    f.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&f);

    settextcolor(txtColor);
    setbkmode(TRANSPARENT);

    // 计算文字居中位置
    int textWidth = textwidth(text.c_str());
    int textHeight = textheight(text.c_str());
    int textX = x + (width - textWidth) / 2;
    int textY = y + (height - textHeight) / 2;

    outtextxy(textX, textY, text.c_str());
}

// ============== UICard 实现 ==============

void UICard::Draw()
{
    if (!visible)
        return;

    // 绘制阴影
    if (hasShadow)
    {
        COLORREF shadowColor = RGB(17, 45, 78);
        setfillcolor(shadowColor);
        setlinecolor(shadowColor);
        solidroundrect(x + shadowOffset, y + shadowOffset,
                       x + width + shadowOffset, y + height + shadowOffset,
                       cornerRadius, cornerRadius);
    }

    // 绘制卡片背景
    setfillcolor(backgroundColor);
    setlinecolor(borderColor);
    solidroundrect(x, y, x + width, y + height, cornerRadius, cornerRadius);

    // 绘制边框
    setlinecolor(borderColor);
    setlinestyle(PS_SOLID, 2);
    roundrect(x, y, x + width, y + height, cornerRadius, cornerRadius);
}

// ============== UIStatusDot 实现 ==============

UIStatusDot::UIStatusDot(int x, int y, int radius, DotStatus status)
    : UIComponent(x - radius, y - radius, radius * 2, radius * 2),
      radius(radius), status(status)
{
}

void UIStatusDot::Draw()
{
    if (!visible)
        return;

    COLORREF dotColor;
    switch (status)
    {
    case DOT_GREEN:
        dotColor = RGB(76, 175, 80); // 绿色
        break;
    case DOT_RED:
        dotColor = RGB(244, 67, 54); // 红色
        break;
    case DOT_GRAY:
        dotColor = RGB(158, 158, 158); // 灰色
        break;
    case DOT_BLUE:
        dotColor = RGB(63, 114, 175); // 蓝色
        break;
    case DOT_YELLOW:
        dotColor = RGB(255, 193, 7); // 黄色
        break;
    default:
        dotColor = RGB(158, 158, 158);
        break;
    }

    setfillcolor(dotColor);
    setlinecolor(dotColor);
    int centerX = x + radius;
    int centerY = y + radius;
    solidcircle(centerX, centerY, radius);
}

// ============== UIPlayerCard 实现 ==============

UIPlayerCard::UIPlayerCard(int x, int y, int width, int height)
    : UIComponent(x, y, width, height),
      isWaiting(true),
      playerName(L""),
      statusText(L"等待中..."),
      avatarColor(RGB(63, 114, 175)),
      borderColor(RGB(219, 226, 239)),
      backgroundColor(RGB(249, 247, 247)),
      cornerRadius(15),
      avatarSize(80)
{
}

void UIPlayerCard::Draw()
{
    if (!visible)
        return;

    // 绘制卡片背景
    setfillcolor(backgroundColor);
    setlinecolor(borderColor);
    setlinestyle(PS_SOLID, 3);
    solidroundrect(x, y, x + width, y + height, cornerRadius, cornerRadius);
    roundrect(x, y, x + width, y + height, cornerRadius, cornerRadius);

    if (isWaiting)
    {
        // 绘制等待状态
        DrawWaitingState();
    }
    else
    {
        // 绘制玩家信息
        DrawPlayerInfo();
    }
}

void UIPlayerCard::DrawWaitingState()
{
    // 绘制虚线边框效果
    setlinecolor(RGB(200, 200, 200));
    setlinestyle(PS_DASH, 2);
    roundrect(x, y, x + width, y + height, cornerRadius, cornerRadius);

    // 绘制占位头像（空心圆）
    int avatarCenterX = x + width / 2;
    int avatarCenterY = y + 100;
    setlinecolor(RGB(200, 200, 200));
    setlinestyle(PS_SOLID, 3);
    circle(avatarCenterX, avatarCenterY, avatarSize / 2);

    // 绘制等待文字
    settextcolor(RGB(150, 150, 150));
    settextstyle(32, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);

    int textWidth = textwidth(statusText.c_str());
    outtextxy(avatarCenterX - textWidth / 2, avatarCenterY + 80, statusText.c_str());
}

void UIPlayerCard::DrawPlayerInfo()
{
    // 绘制头像（实心方块）
    int avatarCenterX = x + width / 2;
    int avatarCenterY = y + 100;
    setfillcolor(avatarColor);
    setlinecolor(avatarColor);
    int halfSize = avatarSize / 2;
    solidrectangle(avatarCenterX - halfSize, avatarCenterY - halfSize,
                   avatarCenterX + halfSize, avatarCenterY + halfSize);

    // 绘制玩家昵称
    settextcolor(RGB(17, 45, 78));
    settextstyle(36, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);

    int nameWidth = textwidth(playerName.c_str());
    outtextxy(avatarCenterX - nameWidth / 2, avatarCenterY + 80, playerName.c_str());

    // 绘制状态文字
    COLORREF statusColor = (statusText == L"已准备") ? RGB(76, 175, 80) : RGB(158, 158, 158);
    settextcolor(statusColor);
    settextstyle(28, 0, L"微软雅黑");

    int statusWidth = textwidth(statusText.c_str());
    outtextxy(avatarCenterX - statusWidth / 2, avatarCenterY + 130, statusText.c_str());
}

// ============== UILoadingAnimation 实现 ==============

UILoadingAnimation::UILoadingAnimation(int x, int y, int size, AnimationType type)
    : UIComponent(x - size, y - size, size * 2, size * 2),
      size(size), type(type), animationFrame(0), maxFrames(60),
      color(RGB(63, 114, 175))
{
}

void UILoadingAnimation::Draw()
{
    if (!visible)
        return;

    int centerX = x + width / 2;
    int centerY = y + height / 2;

    switch (type)
    {
    case ANIM_BREATHING_CIRCLE:
        DrawBreathingCircle(centerX, centerY);
        break;
    case ANIM_RADAR_SCAN:
        DrawRadarScan(centerX, centerY);
        break;
    case ANIM_DOTS_JUMPING:
        DrawDotsJumping(centerX, centerY);
        break;
    }

    // 更新动画帧
    animationFrame = (animationFrame + 1) % maxFrames;
}

void UILoadingAnimation::DrawBreathingCircle(int centerX, int centerY)
{
    // 呼吸圆：半径周期变化
    float phase = (float)animationFrame / maxFrames * 2 * 3.14159f;
    int minRadius = size / 2;
    int maxRadius = size;
    int currentRadius = minRadius + (int)((maxRadius - minRadius) * (0.5f + 0.5f * sin(phase)));

    setlinecolor(color);
    setlinestyle(PS_SOLID, 3);
    circle(centerX, centerY, currentRadius);
}

void UILoadingAnimation::DrawRadarScan(int centerX, int centerY)
{
    // 雷达扫描：扇形旋转
    float startAngle = (float)animationFrame / maxFrames * 2 * 3.14159f;
    float sweepAngle = 3.14159f / 3; // 60度扇形

    setfillcolor(color);
    setlinecolor(color);

    // 绘制扇形（使用 pie 函数）
    int left = centerX - size;
    int top = centerY - size;
    int right = centerX + size;
    int bottom = centerY + size;

    // EasyX 的 pie 函数参数：左上右下坐标 + 起始角度 + 终止角度
    solidpie(left, top, right, bottom, startAngle, startAngle + sweepAngle);
}

void UILoadingAnimation::DrawDotsJumping(int centerX, int centerY)
{
    // 三点跳动：小圆点垂直位置变化
    int dotRadius = 8;
    int dotSpacing = 25;

    for (int i = 0; i < 3; i++)
    {
        float phase = (float)(animationFrame + i * 20) / maxFrames * 2 * 3.14159f;
        int offsetY = (int)(10 * sin(phase));

        int dotX = centerX - dotSpacing + i * dotSpacing;
        int dotY = centerY + offsetY;

        setfillcolor(color);
        setlinecolor(color);
        solidcircle(dotX, dotY, dotRadius);
    }
}
