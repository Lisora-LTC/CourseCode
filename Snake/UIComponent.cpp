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
    case CAPSULE:
        DrawCapsule(bgColor, txtColor);
        break;
    case ROUNDED_RECT:
        DrawRoundedRect(bgColor, txtColor);
        break;
    case RECT:
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
    roundrect(x, y, x + width, y + height, cornerRadius, cornerRadius);
}
