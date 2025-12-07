#pragma once
#include "Common.h"
#include "InputManager.h"
#include <graphics.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <locale>
#include <codecvt>

// ============== 游戏记录结构 ==============
struct HistoryRecord
{
    std::wstring time; // 时间
    std::wstring mode; // 模式
    int score;         // 得分
    int length;        // 长度

    HistoryRecord() : score(0), length(0) {}
    HistoryRecord(const std::wstring &t, const std::wstring &m, int s, int l)
        : time(t), mode(m), score(s), length(l) {}
};

// ============== 历史记录场景 ==============
class HistoryScene
{
private:
    InputManager inputMgr;
    bool sceneRunning;
    bool manageWindow;

    std::vector<HistoryRecord> records;         // 所有记录
    std::vector<HistoryRecord> filteredRecords; // 过滤后的记录

    int currentFilter;                 // 当前选择的模式过滤器 (0=全部, 1=单人, 2=双人)
    int displayStartIndex;             // 显示起始索引（用于分页）
    const int MAX_DISPLAY_RECORDS = 5; // 最多显示5条记录

    // 最高分记录
    HistoryRecord bestRecord;

    // 颜色定义
    const COLORREF COLOR_BG = RGB(249, 247, 247);     // #F9F7F7 米白
    const COLORREF COLOR_BORDER = RGB(219, 226, 239); // #DBE2EF 灰蓝
    const COLORREF COLOR_ACCENT = RGB(63, 114, 175);  // #3F72AF 亮蓝
    const COLORREF COLOR_DARK = RGB(17, 45, 78);      // #112D4E 深藏青
    const COLORREF COLOR_WHITE = RGB(249, 247, 247);  // #F9F7F7 白色

public:
    // ============== 构造与析构 ==============
    HistoryScene(bool manageWindow = false);
    ~HistoryScene();

    // ============== 主方法 ==============
    void Show();

private:
    // ============== 数据加载 ==============
    void LoadRecords();
    void FilterRecords();
    void FindBestRecord();
    std::wstring ParseModeFromString(const std::string &modeStr);
    std::wstring ParseModeFromEnglish(const std::string &modeStr);

    // ============== 渲染方法 ==============
    void Render();
    void DrawBestRecordCard();
    void DrawFilterButtons();
    void DrawRecordsList();
    void DrawFooter();
    void DrawSingleRecord(const HistoryRecord &record, int x, int y, int width, int height);

    // ============== 输入处理 ==============
    void HandleInput();
    bool IsMouseInRect(int mouseX, int mouseY, int x, int y, int width, int height);

    // ============== 工具方法 ==============
    void DrawRoundRect(int x, int y, int width, int height, int radius, COLORREF fillColor, COLORREF borderColor);
    void DrawTrophy(int x, int y, int size);
};
