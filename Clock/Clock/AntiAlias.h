#pragma once
#include <graphics.h>

// 抗锯齿算法命名空间
namespace AntiAlias {
    // 基础颜色混合函数
    COLORREF blendColor(COLORREF bg, COLORREF fg, float alpha);
    
    // 安全获取像素颜色
    COLORREF getPixelSafe(int x, int y, int width, int height);
    
    // 设置带透明度的像素
    void setPixelAlpha(int x, int y, COLORREF color, float alpha, int width, int height);
    
    // Wu's 抗锯齿直线算法
    void drawAntiAliasedLine(int x1, int y1, int x2, int y2, COLORREF color, int thickness = 1, int width = 1280, int height = 960);
    
    // 抗锯齿圆圈绘制
    void drawAntiAliasedCircle(int centerX, int centerY, int radius, COLORREF color, bool filled = true, int width = 1280, int height = 960);
    
    // 2xSSAA (Super Sampling Anti-Aliasing) 支持
    namespace SSAA {
        // 2xSSAA直线绘制 - 在2倍分辨率下绘制后降采样
        void drawLine2xSSAA(int x1, int y1, int x2, int y2, COLORREF color, int thickness = 1, int width = 1280, int height = 960);
        
        // 2xSSAA圆圈绘制
        void drawCircle2xSSAA(int centerX, int centerY, int radius, COLORREF color, bool filled = true, int width = 1280, int height = 960);
        
        // 降采样辅助函数 - 从2x缓冲区采样到目标像素
        COLORREF downsample2x2(COLORREF* buffer, int bufferWidth, int x, int y);
    }
}
