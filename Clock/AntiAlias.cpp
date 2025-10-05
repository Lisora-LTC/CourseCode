#include "AntiAlias.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace AntiAlias {
    // 混合两个颜色，alpha范围0-1
    COLORREF blendColor(COLORREF bg, COLORREF fg, float alpha) {
        if (alpha <= 0) return bg;
        if (alpha >= 1) return fg;
        
        int bgR = GetRValue(bg), bgG = GetGValue(bg), bgB = GetBValue(bg);
        int fgR = GetRValue(fg), fgG = GetGValue(fg), fgB = GetBValue(fg);
        
        int r = (int)(bgR * (1 - alpha) + fgR * alpha);
        int g = (int)(bgG * (1 - alpha) + fgG * alpha);
        int b = (int)(bgB * (1 - alpha) + fgB * alpha);
        
        return RGB(r, g, b);
    }
    
    // 获取指定位置的像素颜色（安全版本）
    COLORREF getPixelSafe(int x, int y, int width, int height) {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            return RGB(239, 246, 255); // 返回背景色
        }
        return getpixel(x, y);
    }
    
    // 设置带透明度的像素
    void setPixelAlpha(int x, int y, COLORREF color, float alpha, int width, int height) {
        if (x < 0 || y < 0 || x >= width || y >= height) return;
        
        COLORREF bgColor = getPixelSafe(x, y, width, height);
        COLORREF blended = blendColor(bgColor, color, alpha);
        putpixel(x, y, blended);
    }
    
    // Wu's 抗锯齿直线算法
    void drawAntiAliasedLine(int x1, int y1, int x2, int y2, COLORREF color, int thickness, int width, int height) {
        // 处理粗线条：绘制多条平行线
        if (thickness > 1) {
            float dx = y2 - y1;
            float dy = x1 - x2;
            float len = sqrt(dx * dx + dy * dy);
            if (len > 0) {
                dx /= len;
                dy /= len;
            }
            
            for (int t = -(thickness / 2); t <= thickness / 2; t++) {
                int offset_x1 = x1 + (int)(t * dx);
                int offset_y1 = y1 + (int)(t * dy);
                int offset_x2 = x2 + (int)(t * dx);
                int offset_y2 = y2 + (int)(t * dy);
                drawAntiAliasedLine(offset_x1, offset_y1, offset_x2, offset_y2, color, 1, width, height);
            }
            return;
        }
        
        // Wu's 算法核心
        bool steep = abs(y2 - y1) > abs(x2 - x1);
        
        if (steep) {
            std::swap(x1, y1);
            std::swap(x2, y2);
        }
        if (x1 > x2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        
        float dx = x2 - x1;
        float dy = y2 - y1;
        float gradient = (dx == 0) ? 1.0f : dy / dx;
        
        // 处理端点
        float xend = x1;
        float yend = y1 + gradient * (xend - x1);
        float xgap = 1 - (x1 + 0.5f - floor(x1 + 0.5f));
        int xpxl1 = (int)xend;
        int ypxl1 = (int)floor(yend);
        
        if (steep) {
            setPixelAlpha(ypxl1, xpxl1, color, (1 - (yend - floor(yend))) * xgap, width, height);
            setPixelAlpha(ypxl1 + 1, xpxl1, color, (yend - floor(yend)) * xgap, width, height);
        } else {
            setPixelAlpha(xpxl1, ypxl1, color, (1 - (yend - floor(yend))) * xgap, width, height);
            setPixelAlpha(xpxl1, ypxl1 + 1, color, (yend - floor(yend)) * xgap, width, height);
        }
        
        float intery = yend + gradient;
        
        xend = x2;
        yend = y2 + gradient * (xend - x2);
        xgap = x2 + 0.5f - floor(x2 + 0.5f);
        int xpxl2 = (int)xend;
        int ypxl2 = (int)floor(yend);
        
        if (steep) {
            setPixelAlpha(ypxl2, xpxl2, color, (1 - (yend - floor(yend))) * xgap, width, height);
            setPixelAlpha(ypxl2 + 1, xpxl2, color, (yend - floor(yend)) * xgap, width, height);
        } else {
            setPixelAlpha(xpxl2, ypxl2, color, (1 - (yend - floor(yend))) * xgap, width, height);
            setPixelAlpha(xpxl2, ypxl2 + 1, color, (yend - floor(yend)) * xgap, width, height);
        }
        
        // 主循环
        for (int x = xpxl1 + 1; x < xpxl2; x++) {
            if (steep) {
                setPixelAlpha((int)floor(intery), x, color, 1 - (intery - floor(intery)), width, height);
                setPixelAlpha((int)floor(intery) + 1, x, color, intery - floor(intery), width, height);
            } else {
                setPixelAlpha(x, (int)floor(intery), color, 1 - (intery - floor(intery)), width, height);
                setPixelAlpha(x, (int)floor(intery) + 1, color, intery - floor(intery), width, height);
            }
            intery += gradient;
        }
    }
    
    // 抗锯齿圆圈绘制
    void drawAntiAliasedCircle(int centerX, int centerY, int radius, COLORREF color, bool filled, int width, int height) {
        for (int y = -radius - 1; y <= radius + 1; y++) {
            for (int x = -radius - 1; x <= radius + 1; x++) {
                float distance = sqrt(x * x + y * y);
                float alpha = 0;
                
                if (filled) {
                    // 实心圆
                    if (distance <= radius - 0.5f) {
                        alpha = 1.0f;
                    } else if (distance < radius + 0.5f) {
                        alpha = radius + 0.5f - distance;
                    }
                } else {
                    // 空心圆（圆环）
                    float innerRadius = radius - 0.5f;
                    float outerRadius = radius + 0.5f;
                    if (distance >= innerRadius && distance <= outerRadius) {
                        if (distance <= radius) {
                            alpha = distance - innerRadius;
                        } else {
                            alpha = outerRadius - distance;
                        }
                    }
                }
                
                if (alpha > 0) {
                    setPixelAlpha(centerX + x, centerY + y, color, alpha, width, height);
                }
            }
        }
    }
    
    // 2xSSAA 实现
    namespace SSAA {
        // 降采样辅助函数 - 从2x2采样区域计算平均颜色
        COLORREF downsample2x2(COLORREF* buffer, int bufferWidth, int x, int y) {
            int r = 0, g = 0, b = 0;
            int count = 0;
            
            // 采样2x2区域
            for (int dy = 0; dy < 2; dy++) {
                for (int dx = 0; dx < 2; dx++) {
                    int idx = (y * 2 + dy) * bufferWidth + (x * 2 + dx);
                    COLORREF c = buffer[idx];
                    r += GetRValue(c);
                    g += GetGValue(c);
                    b += GetBValue(c);
                    count++;
                }
            }
            
            return RGB(r / count, g / count, b / count);
        }
        
        // 2xSSAA直线绘制
        void drawLine2xSSAA(int x1, int y1, int x2, int y2, COLORREF color, int thickness, int width, int height) {
            // 创建2倍分辨率的临时缓冲区
            int bufferWidth = width * 2;
            int bufferHeight = height * 2;
            std::vector<COLORREF> buffer(bufferWidth * bufferHeight);
            
            // 填充背景色
            COLORREF bgColor = RGB(239, 246, 255);
            for (int i = 0; i < bufferWidth * bufferHeight; i++) {
                buffer[i] = bgColor;
            }
            
            // 在2倍分辨率下绘制直线（使用简单的Bresenham算法）
            int x1_2x = x1 * 2;
            int y1_2x = y1 * 2;
            int x2_2x = x2 * 2;
            int y2_2x = y2 * 2;
            int thickness_2x = thickness * 2;
            
            // 使用Bresenham算法绘制线条
            int dx = abs(x2_2x - x1_2x);
            int dy = abs(y2_2x - y1_2x);
            int sx = (x1_2x < x2_2x) ? 1 : -1;
            int sy = (y1_2x < y2_2x) ? 1 : -1;
            int err = dx - dy;
            
            int x = x1_2x;
            int y = y1_2x;
            
            while (true) {
                // 绘制粗线条（通过在周围绘制多个点）
                for (int ty = -thickness_2x / 2; ty <= thickness_2x / 2; ty++) {
                    for (int tx = -thickness_2x / 2; tx <= thickness_2x / 2; tx++) {
                        int px = x + tx;
                        int py = y + ty;
                        if (px >= 0 && px < bufferWidth && py >= 0 && py < bufferHeight) {
                            buffer[py * bufferWidth + px] = color;
                        }
                    }
                }
                
                if (x == x2_2x && y == y2_2x) break;
                
                int e2 = 2 * err;
                if (e2 > -dy) {
                    err -= dy;
                    x += sx;
                }
                if (e2 < dx) {
                    err += dx;
                    y += sy;
                }
            }
            
            // 降采样到目标分辨率
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    COLORREF c = downsample2x2(buffer.data(), bufferWidth, x, y);
                    putpixel(x, y, c);
                }
            }
        }
        
        // 2xSSAA圆圈绘制
        void drawCircle2xSSAA(int centerX, int centerY, int radius, COLORREF color, bool filled, int width, int height) {
            // 创建2倍分辨率的临时缓冲区
            int bufferWidth = width * 2;
            int bufferHeight = height * 2;
            std::vector<COLORREF> buffer(bufferWidth * bufferHeight);
            
            // 填充背景色
            COLORREF bgColor = RGB(239, 246, 255);
            for (int i = 0; i < bufferWidth * bufferHeight; i++) {
                buffer[i] = bgColor;
            }
            
            // 在2倍分辨率下绘制圆圈
            int centerX_2x = centerX * 2;
            int centerY_2x = centerY * 2;
            int radius_2x = radius * 2;
            
            for (int y = -radius_2x - 2; y <= radius_2x + 2; y++) {
                for (int x = -radius_2x - 2; x <= radius_2x + 2; x++) {
                    float distance = sqrt(x * x + y * y);
                    bool shouldDraw = false;
                    
                    if (filled) {
                        shouldDraw = (distance <= radius_2x);
                    } else {
                        shouldDraw = (distance >= radius_2x - 1 && distance <= radius_2x + 1);
                    }
                    
                    if (shouldDraw) {
                        int px = centerX_2x + x;
                        int py = centerY_2x + y;
                        if (px >= 0 && px < bufferWidth && py >= 0 && py < bufferHeight) {
                            buffer[py * bufferWidth + px] = color;
                        }
                    }
                }
            }
            
            // 降采样到目标分辨率
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    COLORREF c = downsample2x2(buffer.data(), bufferWidth, x, y);
                    
                    // 只在圆圈区域内进行混合
                    int dx = x - centerX;
                    int dy = y - centerY;
                    float dist = sqrt(dx * dx + dy * dy);
                    
                    if ((filled && dist <= radius + 1) || (!filled && dist >= radius - 1 && dist <= radius + 1)) {
                        putpixel(x, y, c);
                    }
                }
            }
        }
    }
}
