
#include <graphics.h>   // EasyX
#include <conio.h>
#include <ctime>
#include <cmath>
#include <Windows.h>
#include <algorithm>
#include "AntiAlias.h"
#include "ThemeManager.h"

// ���������ĵ�
#define WIN_W 1280
#define WIN_H 960
const int CENTER_X = WIN_W / 2;
const int CENTER_Y = 400;  // �����ƶ������·����������ռ䣨ԭ���� WIN_H / 2 = 480��

// ����ݣ�1=2xSSAA��0=Wu's
#define USE_2X_SSAA 1
#define USE_2X_SSAA_FOR_STATIC 1  

// ���ڶ�̬Ԫ�أ�ָ�룩
#if USE_2X_SSAA
    #define AA_DrawLine(x1, y1, x2, y2, color, thickness) \
        AntiAlias::SSAA::drawLine2xSSAA(x1, y1, x2, y2, color, thickness, WIN_W, WIN_H)
    #define AA_DrawCircle(cx, cy, r, color, filled) \
        AntiAlias::SSAA::drawCircle2xSSAA(cx, cy, r, color, filled, WIN_W, WIN_H)
#else
    #define AA_DrawLine(x1, y1, x2, y2, color, thickness) \
        AntiAlias::drawAntiAliasedLine(x1, y1, x2, y2, color, thickness, WIN_W, WIN_H)
    #define AA_DrawCircle(cx, cy, r, color, filled) \
        AntiAlias::drawAntiAliasedCircle(cx, cy, r, color, filled, WIN_W, WIN_H)
#endif

// ��̬Ԫ�أ��̶�/�߿�/���ĵ㣩
#if USE_2X_SSAA_FOR_STATIC
    #define AA_DrawLine_Static(x1, y1, x2, y2, color, thickness) \
        AntiAlias::SSAA::drawLine2xSSAA(x1, y1, x2, y2, color, thickness, WIN_W, WIN_H)
    #define AA_DrawCircle_Static(cx, cy, r, color, filled) \
        AntiAlias::SSAA::drawCircle2xSSAA(cx, cy, r, color, filled, WIN_W, WIN_H)
#else
    #define AA_DrawLine_Static(x1, y1, x2, y2, color, thickness) \
        AntiAlias::drawAntiAliasedLine(x1, y1, x2, y2, color, thickness, WIN_W, WIN_H)
    #define AA_DrawCircle_Static(cx, cy, r, color, filled) \
        AntiAlias::drawAntiAliasedCircle(cx, cy, r, color, filled, WIN_W, WIN_H)
#endif

// ʱ��ָ����
class ClockHand {
private:
    int centerX;
    int centerY;
    double length;
    double angle;   // ���ȣ�0 ָ��12��
    COLORREF color;
    int thickness;

public:
    ClockHand(int cx = CENTER_X, int cy = CENTER_Y, double len = 100, 
              double ang = 0, COLORREF col = WHITE, int thick = 2)
        : centerX(cx), centerY(cy), length(len), angle(ang), 
          color(col), thickness(thick) {}

    void setCenter(int x, int y) { centerX = x; centerY = y; }
    void setLength(double len) { length = len; }
    void setAngle(double ang) { angle = ang; }
    void setColor(COLORREF col) { color = col; }
    void setThickness(int thick) { thickness = thick; }

    double getAngle() const { return angle; }
    double getLength() const { return length; }
    COLORREF getColor() const { return color; }

    void setTimeAngle(int value, int maxValue) {
        angle = (value * 2.0 * 3.14159265359 / maxValue) - 3.14159265359 / 2.0;
    }

    void render() const {
        int endX = centerX + (int)(length * cos(angle));
        int endY = centerY + (int)(length * sin(angle));

        // ��ɫ��ߣ�������ǿ�Աȣ�
        if (thickness >= 3) {
            AA_DrawLine(centerX, centerY, endX, endY, RGB(255, 255, 255), thickness + 2);
        }

        // ����
        AA_DrawLine(centerX, centerY, endX, endY, color, thickness);

        // Բ��˵�
        if (thickness > 1) {
            AA_DrawCircle(endX, endY, 1, color, true);
        }
    }

    void clear(COLORREF bgColor = BLACK) const {
        int endX = centerX + (int)(length * cos(angle));
        int endY = centerY + (int)(length * sin(angle));

        setlinecolor(bgColor);
        setlinestyle(PS_SOLID, thickness);
        line(centerX, centerY, endX, endY);
    }
};

// ȫ��ָ����󣨳�ʼ��ɫ����updateTheme�и��£�

// ========== ����ģʽ���Ʊ��� ==========
bool isAutoMode = true;      // �Ƿ�Ϊ�Զ�ģʽ������ʱ���л���
bool isNightMode = false;    // ��ǰ�Ƿ�Ϊҹ��ģʽ���ֶ�ģʽ��ʹ�ã�

// ��ťλ�úʹ�С
const int BTN_X = WIN_W - 150;  // ��ť���Ͻ�X����
const int BTN_Y = 20;           // ��ť���Ͻ�Y����
const int BTN_WIDTH = 120;      // ��ť���
const int BTN_HEIGHT = 40;      // ��ť�߶�
// =========================================

// ȫ��ָ����󣨳�ʼ��ɫ���� updateThemeAndHands �и��£�
ClockHand hourHand(CENTER_X, CENTER_Y, 150, 0, RGB(0,0,0), 10);     // ʱ��
ClockHand minuteHand(CENTER_X, CENTER_Y, 230, 0, RGB(0,0,0), 6);    // ����
ClockHand secondHand(CENTER_X, CENTER_Y, 270, 0, RGB(0,0,0), 4);    // ����

// ��װ�������������Ⲣͬ��ָ����ɫ
void updateThemeAndHands(int hour) {
    updateTheme(hour);  // ���� ThemeManager �ĺ�����������
    // ����ָ����ɫ
    hourHand.setColor(currentTheme.hourHandColor);
    minuteHand.setColor(currentTheme.minuteHandColor);
    secondHand.setColor(currentTheme.secondHandColor);
}

// �ֶ��л�����ģʽ��������ʱ�䣩
void switchThemeManually(bool toNightMode) {
    isNightMode = toNightMode;
    currentTheme = isNightMode ? NIGHT_THEME : DAY_THEME;
    // ����ָ����ɫ
    hourHand.setColor(currentTheme.hourHandColor);
    minuteHand.setColor(currentTheme.minuteHandColor);
    secondHand.setColor(currentTheme.secondHandColor);
}

// ����ģʽ�л���ť
void drawModeButton() {
    // ��������ѡ��ť��ɫ
    COLORREF btnBg = isAutoMode ? RGB(100, 150, 220) : 
                     (isNightMode ? RGB(80, 100, 130) : RGB(150, 180, 220));
    COLORREF btnBorder = isAutoMode ? RGB(80, 120, 200) : 
                         (isNightMode ? RGB(120, 140, 180) : RGB(120, 150, 200));
    COLORREF btnText = RGB(255, 255, 255);
    
    // ���ư�ť����
    setfillcolor(btnBg);
    setlinecolor(btnBorder);
    setlinestyle(PS_SOLID, 2);
    fillroundrect(BTN_X, BTN_Y, BTN_X + BTN_WIDTH, BTN_Y + BTN_HEIGHT, 10, 10);
    
    // ���ư�ť����
    settextcolor(btnText);
    settextstyle(18, 0, L"΢���ź�");
    setbkmode(TRANSPARENT);
    
    wchar_t btnTextStr[20];
    if (isAutoMode) {
        wcscpy_s(btnTextStr, L"�Զ�ģʽ");
    } else {
        wcscpy_s(btnTextStr, isNightMode ? L"ҹ��ģʽ" : L"�ռ�ģʽ");
    }
    
    // �������־���λ��
    int textWidth = textwidth(btnTextStr);
    int textHeight = textheight(btnTextStr);
    int textX = BTN_X + (BTN_WIDTH - textWidth) / 2;
    int textY = BTN_Y + (BTN_HEIGHT - textHeight) / 2;
    
    outtextxy(textX, textY, btnTextStr);
}

// ��ⰴť���
bool isButtonClicked(int mouseX, int mouseY) {
    return (mouseX >= BTN_X && mouseX <= BTN_X + BTN_WIDTH &&
            mouseY >= BTN_Y && mouseY <= BTN_Y + BTN_HEIGHT);
}

// ǰ������
void init();            // ��ʼ��ʱ�ӽ���
void drawClock();       // ����ʱ�ӱ���
void drawHands(int hour, int minute, int second);  // ����ʱ������
void applyVignette();   // Ӧ�ð���Ч��
void drawShadowCircle(int cx, int cy, int r, COLORREF color);  // �������ͶӰԲ


// ��ʼ������ʵ�� - ʹ�õ�ǰ������ɫ
void init() {
    const int R_MAIN = 310;
    setbkcolor(currentTheme.bgColor);  // ʹ�����ⱳ��ɫ
    cleardevice();                     // ����
    
    // ���ǣ��ײ㣩
    applyVignette();
    
    // ����΢��Ȧ��
    for (int r = 360; r > 0; r -= 40) {
        int alpha = 255 - (r * 50 / 360);
        // ��ȡ��ǰ���ⱳ��ɫ��RGB����
        int bgR = GetRValue(currentTheme.bgColor);
        int bgG = GetGValue(currentTheme.bgColor);
        int bgB = GetBValue(currentTheme.bgColor);
        // ������΢�����Ľ���
        setfillcolor(RGB(
            min(255, bgR + alpha/20),
            min(255, bgG + alpha/20),
            min(255, bgB + alpha/20)
        ));
        setlinecolor(getfillcolor());
        fillcircle(CENTER_X, CENTER_Y, r);
    }
    
    // ������ͶӰ
    drawShadowCircle(CENTER_X, CENTER_Y, R_MAIN, RGB(0, 0, 0));
    
    // ���̾��򽥱�
    for (int r = R_MAIN; r > 0; r -= 2) {
        float t = (float)r / R_MAIN;
        
        // �������ȡ����ɫ
        int centerR = GetRValue(currentTheme.dialCenterColor);
        int centerG = GetGValue(currentTheme.dialCenterColor);
        int centerB = GetBValue(currentTheme.dialCenterColor);
        int edgeR = GetRValue(currentTheme.dialEdgeColor);
        int edgeG = GetGValue(currentTheme.dialEdgeColor);
        int edgeB = GetBValue(currentTheme.dialEdgeColor);
        
        // ���Բ�ֵ
        int r_color = (int)(edgeR + (centerR - edgeR) * t);
        int g_color = (int)(edgeG + (centerG - edgeG) * t);
        int b_color = (int)(edgeB + (centerB - edgeB) * t);
        
        setfillcolor(RGB(r_color, g_color, b_color));
        setlinecolor(RGB(r_color, g_color, b_color));
        solidcircle(CENTER_X, CENTER_Y, r);
    }
    
    // ���̱߿򣨾�̬AA��
    // ʹ���������ڵ�AAԲ�γɽ���3px�ı߿�Ч��
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, R_MAIN, currentTheme.borderColor, false);
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, R_MAIN - 1, currentTheme.borderColor, false);
    
    // ����Сʱ�̶����֣�ʹ������������ɫ��
    for (int i = 0; i < 12; i++) {
        double angle = i * 3.14159265359 / 6 - 3.14159265359 / 2;
        
        settextcolor(currentTheme.textColor);
        setbkmode(TRANSPARENT);
        
        // 12, 3, 6, 9ʹ�ô����壬����ʹ��С����
        if (i % 3 == 0) {
            settextstyle(32, 0, _T("Segoe UI"));
        } else {
            settextstyle(24, 0, _T("Segoe UI"));
        }
        
        int displayNum = (i == 0) ? 12 : i;
        wchar_t num[4];
        swprintf_s(num, L"%d", displayNum);
        
        // ��������λ�ã���Ҫ�������ֿ�ȵ�����- �Ƶ��������ⱻָ���ڵ�
        int textWidth = (displayNum >= 10) ? 18 : 9;
        int textHeight = 14;
        int numX = CENTER_X + (int)(R_MAIN * 0.85 * cos(angle)) - textWidth;
        int numY = CENTER_Y + (int)(R_MAIN * 0.85 * sin(angle)) - textHeight;
        
        outtextxy(numX, numY, num);
    }
    
    // ��������װ��Բ��ʹ����drawHandsһ�µ���ʽ��
    // �Ȼ���ͬ����Ӱ�㣨�Ӵ�С���𽥱䰵��
    for (int i = 3; i > 0; i--) {
        float alphaRatio = 0.1f * i / 3.0f;
        int bgR = GetRValue(currentTheme.bgColor);
        int bgG = GetGValue(currentTheme.bgColor);
        int bgB = GetBValue(currentTheme.bgColor);
        int shadowR = isNightMode ? 0 : 20;
        int shadowG = isNightMode ? 0 : 20;
        int shadowB = isNightMode ? 0 : 30;
        int mixR = (int)(bgR * (1 - alphaRatio) + shadowR * alphaRatio);
        int mixG = (int)(bgG * (1 - alphaRatio) + shadowG * alphaRatio);
        int mixB = (int)(bgB * (1 - alphaRatio) + shadowB * alphaRatio);
        setfillcolor(RGB(mixR, mixG, mixB));
        setlinecolor(RGB(mixR, mixG, mixB));
        solidcircle(CENTER_X, CENTER_Y, 8 + i);
    }
    // �������ĵ�����
    AA_DrawCircle_Static(CENTER_X, CENTER_Y, 8, currentTheme.centerDotColor, true);
    
    // ģʽ�л���ť
    drawModeButton();
}

// ���ǣ�Vignette��
void applyVignette() {
    // ���ǲ������ڱ���֮ǰ���ƣ����Ը����ǿ��
    const int vignetteWidth = 140;  // ���ǿ�ȣ����أ�- ����140
    const int maxAlpha = isNightMode ? 70 : 50;  // ���͸���� - ��ǿЧ��
    
    int bgR = GetRValue(currentTheme.bgColor);
    int bgG = GetGValue(currentTheme.bgColor);
    int bgB = GetBValue(currentTheme.bgColor);
    
    // �������ڻ��ƽ��䣨��Ե������ڽ�����
    // �����ؾ�ϸ���䣬����䱣��һ��
    for (int dist = 0; dist < vignetteWidth; dist++) {
        // dist=0 ʱ������ߣ������dist=vignetteWidth-1 ʱ�����ڲࣨ������
        float t = (float)dist / vignetteWidth;  // 0��1�����⵽��
        float alpha = maxAlpha * (1.0f - t) * (1.0f - t) / 255.0f;  // ��వ���ڲ���
        
        // ��Ϻ�ɫ�ͱ���ɫ��alpha Խ��Խ����
        int r_mix = (int)(bgR * (1.0f - alpha));
        int g_mix = (int)(bgG * (1.0f - alpha));
        int b_mix = (int)(bgB * (1.0f - alpha));
        
        COLORREF vignetteColor = RGB(r_mix, g_mix, b_mix);
        setfillcolor(vignetteColor);
        setlinecolor(vignetteColor);
        
        // ���������ߵĽ��䣨���������䣬�����ػ���1���ؿ���ߣ�
        // �ϱߣ��ų����ҽǣ�
        line(vignetteWidth, dist, WIN_W - vignetteWidth - 1, dist);
        // �±ߣ��ų����ҽǣ�
        line(vignetteWidth, WIN_H - dist - 1, WIN_W - vignetteWidth - 1, WIN_H - dist - 1);
        // ��ߣ��ų����½ǣ�
        line(dist, vignetteWidth, dist, WIN_H - vignetteWidth - 1);
        // �ұߣ��ų����½ǣ�
        line(WIN_W - dist - 1, vignetteWidth, WIN_W - dist - 1, WIN_H - vignetteWidth - 1);
    }
    
    // ���佥�䣨���ı��νӣ�
    // ������Ҫ�����������������ʹ�õ��ڲ����̾���ȷ�����ı��ν�
    for (int x = 0; x < vignetteWidth; x++) {
        for (int y = 0; y < vignetteWidth; y++) {
            // ʹ�õ��ڲ����̾��루min(x,y)����ȷ���ڱ߽紦���ı߽���һ��
            float dist = (float)min(x, y);
            float t = dist / vignetteWidth;  // ��һ���� 0-1
            
            // ����͸���ȣ����ı���ȫһ�£�
            float alpha = maxAlpha * (1.0f - t) * (1.0f - t) / 255.0f;
            
            int r_mix = (int)(bgR * (1.0f - alpha));
            int g_mix = (int)(bgG * (1.0f - alpha));
            int b_mix = (int)(bgB * (1.0f - alpha));
            
            COLORREF vignetteColor = RGB(r_mix, g_mix, b_mix);
            
            setfillcolor(vignetteColor);
            setlinecolor(vignetteColor);
            // ���Ͻ�
            putpixel(x, y, vignetteColor);
            // ���Ͻ�
            putpixel(WIN_W - x - 1, y, vignetteColor);
            // ���½�
            putpixel(x, WIN_H - y - 1, vignetteColor);
            // ���½�
            putpixel(WIN_W - x - 1, WIN_H - y - 1, vignetteColor);
        }
    }
}

// ��ͶӰ����
void drawShadowCircle(int cx, int cy, int r, COLORREF color) {
    // ���ƶ��ģ��ͶӰ���򵥵���ͶӰʵ�֣�
    const int shadowLayers = 5;      // ͶӰ����
    const int shadowOffsetX = 3;     // ͶӰXƫ��
    const int shadowOffsetY = 3;     // ͶӰYƫ��
    
    // ������㿪ʼ���ƣ��𽥱���
    for (int i = shadowLayers; i > 0; i--) {
        // ����͸���ȣ�ͨ���뱳��ɫ���ģ�⣩
        float alphaRatio = 0.15f * i / shadowLayers;  // ÿ��͸���ȵݼ�
        
        // ��ȡ����ɫ��ͶӰɫ
        int bgR = GetRValue(currentTheme.bgColor);
        int bgG = GetGValue(currentTheme.bgColor);
        int bgB = GetBValue(currentTheme.bgColor);
        
        // ͶӰɫΪ���ɫ/��ɫ
        int shadowR = isNightMode ? 0 : 20;
        int shadowG = isNightMode ? 0 : 20;
        int shadowB = isNightMode ? 0 : 30;
        
        // ��ϱ���ɫ��ͶӰɫ
        int mixR = (int)(bgR * (1 - alphaRatio) + shadowR * alphaRatio);
        int mixG = (int)(bgG * (1 - alphaRatio) + shadowG * alphaRatio);
        int mixB = (int)(bgB * (1 - alphaRatio) + shadowB * alphaRatio);
        
        COLORREF shadowColor = RGB(mixR, mixG, mixB);
        
        // ����ͶӰԲ��ÿ���Դ�һ�㣬ƫ�����Դ�һ�㣩
        int offsetScale = i;
        setfillcolor(shadowColor);
        setlinecolor(shadowColor);
        solidcircle(cx + shadowOffsetX * offsetScale / shadowLayers,
                   cy + shadowOffsetY * offsetScale / shadowLayers,
                   r + i - 1);
    }
}

// ��������ʱ����ʾ
void drawDigitalTime(int hour, int minute, int second) {
    // �ײ�����ʱ��������
    const int DIGITAL_Y = WIN_H - 180;  // ����ײ���λ�ã�������ռ�
    
    // ��ײ�����
    setfillcolor(currentTheme.bgColor);
    setlinecolor(currentTheme.bgColor);
    solidrectangle(0, DIGITAL_Y - 20, WIN_W, WIN_H);
    
    // ���»��Ƶײ����ǣ���Ϊ������ˣ�- �������Ǻ�������һ��
    const int vignetteWidth = 140;
    const int maxAlpha = isNightMode ? 70 : 50;
    int bg_r = GetRValue(currentTheme.bgColor);
    int bg_g = GetGValue(currentTheme.bgColor);
    int bg_b = GetBValue(currentTheme.bgColor);
    
    // �����ؾ�ϸ����
    for (int dist = 0; dist < vignetteWidth; dist++) {
        float t = (float)dist / vignetteWidth;
        float alpha = maxAlpha * (1.0f - t) * (1.0f - t) / 255.0f;
        
        int r_mix = (int)(bg_r * (1.0f - alpha));
        int g_mix = (int)(bg_g * (1.0f - alpha));
        int b_mix = (int)(bg_b * (1.0f - alpha));
        
        COLORREF vignetteColor = RGB(r_mix, g_mix, b_mix);
        setlinecolor(vignetteColor);
        
        // ֻ�ػ�ײ�����İ��ǣ�����������Χ�ڣ�
        int bottomY = WIN_H - dist - 1;
        if (bottomY >= DIGITAL_Y - 20) {
            // �ױߣ��ų����ҽǣ�
            line(vignetteWidth, bottomY, WIN_W - vignetteWidth - 1, bottomY);
        }
        
        // ��ߺ��ұߣ����������򶥲����ײ���
        if (dist < vignetteWidth) {
            int leftTopY = max(DIGITAL_Y - 20, vignetteWidth);
            int leftBottomY = WIN_H - vignetteWidth - 1;
            if (leftBottomY >= DIGITAL_Y - 20) {
                // ���
                line(dist, leftTopY, dist, leftBottomY);
                // �ұ�
                line(WIN_W - dist - 1, leftTopY, WIN_W - dist - 1, leftBottomY);
            }
        }
    }
    
    // �ײ����临ԭ
    for (int x = 0; x < vignetteWidth; x++) {
        for (int y = 0; y < vignetteWidth; y++) {
            // ʹ�õ��ڲ����̾��루min(x,y)�������ı��ν�
            float dist = (float)min(x, y);
            float t = dist / vignetteWidth;
            
            float alpha = maxAlpha * (1.0f - t) * (1.0f - t) / 255.0f;
            
            int r_mix = (int)(bg_r * (1.0f - alpha));
            int g_mix = (int)(bg_g * (1.0f - alpha));
            int b_mix = (int)(bg_b * (1.0f - alpha));
            
            COLORREF vignetteColor = RGB(r_mix, g_mix, b_mix);
            
            // ֻ�ػ汻����ĵײ�����
            int bottomStartY = DIGITAL_Y - 20;
            
            // ���½ǣ�ֻ���� Y >= bottomStartY �Ĳ��֣�
            int leftY = WIN_H - y - 1;
            if (leftY >= bottomStartY) {
                putpixel(x, leftY, vignetteColor);
            }
            
            // ���½ǣ�ֻ���� Y >= bottomStartY �Ĳ��֣�
            int rightY = WIN_H - y - 1;
            if (rightY >= bottomStartY) {
                putpixel(WIN_W - x - 1, rightY, vignetteColor);
            }
        }
    }
    
    // ����ʱ��
    settextcolor(currentTheme.textColor);
    setbkmode(TRANSPARENT);
    settextstyle(72, 0, _T("Consolas"));  // �����ֺŵ�72
    
    // ��ʽ��ʱ���ַ��� HH:MM:SS
    wchar_t timeStr[12];
    swprintf_s(timeStr, L"%02d:%02d:%02d", hour, minute, second);
    
    // ��ȷ�������ֿ�Ȳ�����
    int timeWidth = textwidth(timeStr);
    int timeX = CENTER_X - timeWidth / 2;
    
    outtextxy(timeX, DIGITAL_Y, timeStr);
    
    // ������Ϣ
    settextstyle(28, 0, _T("Microsoft YaHei UI"));  // ʹ��΢���źڣ�����28��
    // ʹ������������ɫ��80%͸���Ȱ汾��ͨ��RGB���ģ�⣩
    int textR = GetRValue(currentTheme.textColor);
    int textG = GetGValue(currentTheme.textColor);
    int textB = GetBValue(currentTheme.textColor);
    int bgR = GetRValue(currentTheme.bgColor);
    int bgG = GetGValue(currentTheme.bgColor);
    int bgB = GetBValue(currentTheme.bgColor);
    settextcolor(RGB(
        (textR * 6 + bgR * 4) / 10,  // 60%����ɫ + 40%����ɫ
        (textG * 6 + bgG * 4) / 10,
        (textB * 6 + bgB * 4) / 10
    ));
    
    // ��ȡ��ǰ����
    time_t now = time(NULL);
    struct tm t;
    localtime_s(&t, &now);
    
    wchar_t dateStr[50];
    const wchar_t* weekdays[] = {
        L"\u661F\u671F\u65E5",  // ������
        L"\u661F\u671F\u4E00",  // ����һ
        L"\u661F\u671F\u4E8C",  // ���ڶ�
        L"\u661F\u671F\u4E09",  // ������
        L"\u661F\u671F\u56DB",  // ������
        L"\u661F\u671F\u4E94",  // ������
        L"\u661F\u671F\u516D"   // ������
    };
    swprintf_s(dateStr, L"%04d\u5E74%02d\u6708%02d\u65E5  %s", 
               t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, weekdays[t.tm_wday]);
    
    // ��ȷ�������ڿ�Ȳ�����
    int dateWidth = textwidth(dateStr);
    int dateX = CENTER_X - dateWidth / 2;
    outtextxy(dateX, DIGITAL_Y + 90, dateStr);  // ������ֱ���
}

// ȫ�ֱ�������¼��һ�ε�ʱ�䣬���ⲻ��Ҫ���ػ�
static int lastHour = -1, lastMinute = -1, lastSecond = -1;
static bool needFullRedraw = true;

// ����ʱ�����뺯��ʵ�� - ����˸�汾
void drawHands(int hour, int minute, int second) {
    // ʱ�����ж�
    bool timeChanged = (hour != lastHour || minute != lastMinute || second != lastSecond);
    
    // δ���������ȫ�� �� ����
    if (!timeChanged && !needFullRedraw) {
        return;
    }
    
    // ȫ���ػ�
    if (needFullRedraw) {
        init();
        needFullRedraw = false;
    } else {
    // �ֲ����ָ�븲�����򣨾��򽥱䣩
        const int R_MAIN = 310;
        const int CLEAR_RADIUS = 280;  // ����뾶����������ȫ��(270+10)
        
        // �þ��򽥱��ػ�ָ������ʹ��������ɫ��
        for (int r = CLEAR_RADIUS; r > 0; r -= 2) {
            float t = (float)r / R_MAIN;
            
            int centerR = GetRValue(currentTheme.dialCenterColor);
            int centerG = GetGValue(currentTheme.dialCenterColor);
            int centerB = GetBValue(currentTheme.dialCenterColor);
            int edgeR = GetRValue(currentTheme.dialEdgeColor);
            int edgeG = GetGValue(currentTheme.dialEdgeColor);
            int edgeB = GetBValue(currentTheme.dialEdgeColor);
            
            int r_color = (int)(edgeR + (centerR - edgeR) * t);
            int g_color = (int)(edgeG + (centerG - edgeG) * t);
            int b_color = (int)(edgeB + (centerB - edgeB) * t);
            
            setfillcolor(RGB(r_color, g_color, b_color));
            setlinecolor(RGB(r_color, g_color, b_color));
            solidcircle(CENTER_X, CENTER_Y, r);
        }
        
    // ��ԭ�̶�����
        for (int i = 0; i < 12; i++) {
            double angle = i * 3.14159265359 / 6 - 3.14159265359 / 2;
            
            settextcolor(currentTheme.textColor);
            setbkmode(TRANSPARENT);
            
            if (i % 3 == 0) {
                settextstyle(32, 0, _T("Segoe UI"));
            } else {
                settextstyle(24, 0, _T("Segoe UI"));
            }
            
            int displayNum = (i == 0) ? 12 : i;
            wchar_t num[4];
            swprintf_s(num, L"%d", displayNum);
            
            int textWidth = (displayNum >= 10) ? 18 : 9;
            int textHeight = 14;
            int numX = CENTER_X + (int)(R_MAIN * 0.85 * cos(angle)) - textWidth;
            int numY = CENTER_Y + (int)(R_MAIN * 0.85 * sin(angle)) - textHeight;
            
            outtextxy(numX, numY, num);
        }
    }
    
    // ����ָ��ǶȲ���Ⱦ
    hourHand.setTimeAngle((hour % 12) * 5 + minute / 12, 60);
    minuteHand.setTimeAngle(minute, 60);
    secondHand.setTimeAngle(second, 60);
    
    // ��Ⱦָ��
    hourHand.render();
    minuteHand.render();
    secondHand.render();
    
    // ���ĵ㣺ͬ����Ӱ + ʵ������
    for (int i = 3; i > 0; i--) {
        float alphaRatio = 0.1f * i / 3.0f;
        int bgR = GetRValue(currentTheme.bgColor);
        int bgG = GetGValue(currentTheme.bgColor);
        int bgB = GetBValue(currentTheme.bgColor);
        int shadowR = isNightMode ? 0 : 20;
        int shadowG = isNightMode ? 0 : 20;
        int shadowB = isNightMode ? 0 : 30;
        int mixR = (int)(bgR * (1 - alphaRatio) + shadowR * alphaRatio);
        int mixG = (int)(bgG * (1 - alphaRatio) + shadowG * alphaRatio);
        int mixB = (int)(bgB * (1 - alphaRatio) + shadowB * alphaRatio);
        setfillcolor(RGB(mixR, mixG, mixB));
        setlinecolor(RGB(mixR, mixG, mixB));
        solidcircle(CENTER_X, CENTER_Y, 8 + i);
    }
    // �������ĵ�����
    AA_DrawCircle(CENTER_X, CENTER_Y, 8, currentTheme.centerDotColor, true);
    
    // ��������ʱ����ʾ
    drawDigitalTime(hour, minute, second);
    
    // ����ʱ���¼
    lastHour = hour;
    lastMinute = minute;
    lastSecond = second;
}

int main()
{
    // ��ȫ���ؿ���̨����
    HWND hwndConsole = GetConsoleWindow();
    if (hwndConsole != NULL) {
        ShowWindow(hwndConsole, SW_HIDE);
        // �ͷſ���̨
        FreeConsole();
    }
    
    struct tm t;
    time_t now;
    time(&now);
    localtime_s(&t, &now);	// ��ȡ����ʱ��
    
    // ��ʼ������
    updateThemeAndHands(t.tm_hour);
    
    // ��ʼ��ͼ�δ��ڣ����ؿ���̨����
    initgraph(WIN_W, WIN_H);
    
    // ���ô��ڱ���
    SetWindowText(GetHWnd(), L"Modern Clock - Day/Night Mode");
    
    // ����˫���壬��ֹ��˸
    BeginBatchDraw();
    
    // ���ÿ���ݺ�ƽ��Ч��  
    setbkmode(TRANSPARENT);  // ����͸������ģʽ�������ڿ����
    
    init();			// �Զ���ͼ�γ�ʼ�����������ڻ���ʱ�ӽ���
    drawDigitalTime(t.tm_hour, t.tm_min, t.tm_sec);  // ��ʼ��ʱ��ʾ����ʱ��

    // ������Ϣѭ����֧�ִ��ڹر�
    ExMessage msg;
    int previousHour = t.tm_hour;  // ��¼��һ�ε�Сʱ�����ڼ�������л�ʱ��
    
    while (true)
    {
        // ����Ƿ��д�����Ϣ���������д��������Ϣ��
        while (peekmessage(&msg, EX_MOUSE | EX_KEY | EX_WINDOW))
        {
            if (msg.message == WM_CLOSE)  // ����رհ�ť
            {
                goto exit_loop;  // �˳����ѭ��
            }
            
            // ��ť���
            if (msg.message == WM_LBUTTONDOWN) {  // ����������
                if (isButtonClicked(msg.x, msg.y)) {
                    // ��̬ѭ���л����Զ�ģʽ -> �ռ�ģʽ -> ҹ��ģʽ -> �Զ�ģʽ
                    if (isAutoMode) {
                        // �Զ� -> �ռ�
                        isAutoMode = false;
                        isNightMode = false;
                        switchThemeManually(false);
                    } else if (!isNightMode) {
                        // �ռ� -> ҹ��
                        isNightMode = true;
                        switchThemeManually(true);
                    } else {
                        // ҹ�� -> �Զ�
                        isAutoMode = true;
                        updateThemeAndHands(t.tm_hour);
                    }
                    
                    // ���»�����������
                    needFullRedraw = true;
                    init();
                    drawDigitalTime(t.tm_hour, t.tm_min, t.tm_sec);
                    FlushBatchDraw();
                }
            }
        }
        
        /* ��ͣ��ȡ��ǰʱ�䣬��֮ǰ����ʱ/��/����ʱ��ʱ����ȣ�
        ���ж���ʱ��ĵ��� */
        time(&now);
        localtime_s(&t, &now);
        
    // Сʱ������Զ�������
        if (isAutoMode && t.tm_hour != previousHour) {
            // ����Ƿ���Ҫ�л����⣨6���18�㣩
            if ((previousHour == 5 && t.tm_hour == 6) ||      // ����6���л����ռ�
                (previousHour == 17 && t.tm_hour == 18)) {    // ����18���л���ҹ��
                updateThemeAndHands(t.tm_hour);
                needFullRedraw = true;  // �����Ҫ��ȫ�ػ�
            }
            previousHour = t.tm_hour;
        }
        
        /* �����µ�ʱ�����»���ʱ�룬���룬���룬
        �����Զ��庯����ɣ�ע�������������� */
        drawHands(t.tm_hour, t.tm_min, t.tm_sec);
        
        // ˢ����ʾ����������ֹ��˸
        FlushBatchDraw();
        
        Sleep(100);  // �������ռ��CPU
    }
    
exit_loop:  // �˳���ǩ
    EndBatchDraw();     // ����˫����
    closegraph();		// �˳�ͼ�ν���
    return 0;
}