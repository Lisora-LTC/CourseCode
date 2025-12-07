#include <winsock2.h>
#include "HistoryScene.h"
#include <windows.h>
#include <sstream>
#include <algorithm>

// ============== 构造与析构 ==============
HistoryScene::HistoryScene(bool manageWindow)
    : sceneRunning(false), manageWindow(manageWindow), currentFilter(0), displayStartIndex(0)
{
    LoadRecords();
}

HistoryScene::~HistoryScene()
{
    if (manageWindow)
    {
        closegraph();
    }
}

// ============== 主方法 ==============
void HistoryScene::Show()
{
    if (manageWindow)
    {
        initgraph(1920, 1080);
    }

    setbkcolor(COLOR_BG);
    cleardevice();

    sceneRunning = true;

    while (sceneRunning)
    {
        if (inputMgr.IsWindowClosed())
        {
            exit(0);
        }

        inputMgr.Update();
        HandleInput();
        Render();
        Sleep(10);
    }

    if (manageWindow)
    {
        closegraph();
    }
}

// ============== 数据加载 ==============
void HistoryScene::LoadRecords()
{
    records.clear();

    // 尝试多个可能的路径
    const char *possiblePaths[] = {
        "game_records.txt",         // 当前目录（直接运行源码目录下的exe）
        "..\\..\\game_records.txt", // VS项目：x64\Debug 或 x64\Release 往上两级
        "..\\game_records.txt"      // 备用：往上一级
    };

    std::ifstream inFile;
    bool fileFound = false;
    std::string usedPath;

    for (const char *path : possiblePaths)
    {
        inFile.open(path);
        if (inFile.is_open())
        {
            fileFound = true;
            usedPath = path;
            break;
        }
    }

    if (!fileFound)
    {
        // 文件未找到，返回空记录
        return;
    }

    std::string line;
    while (std::getline(inFile, line))
    {
        if (line.empty())
            continue;

        // 新格式: 2025-12-05 23:48:09,SINGLE,40,5
        // 或旧格式: 时间:2025-12-05 23:48:09 模式:单人模式 得分:40 长度:5
        std::istringstream iss(line);

        // 检测是新格式还是旧格式
        if (line.find(',') != std::string::npos)
        {
            // 新格式：CSV格式，逗号分隔
            std::string timeStr, modeStr, scoreStr, lengthStr;

            std::getline(iss, timeStr, ',');
            std::getline(iss, modeStr, ',');
            std::getline(iss, scoreStr, ',');
            std::getline(iss, lengthStr);

            try
            {
                int score = std::stoi(scoreStr);
                int length = std::stoi(lengthStr);

                // 转换为宽字符
                std::wstring wTime(timeStr.begin(), timeStr.end());
                std::wstring wMode = ParseModeFromEnglish(modeStr);

                records.push_back(HistoryRecord(wTime, wMode, score, length));
            }
            catch (...)
            {
                continue;
            }
        }
        else
        {
            // 旧格式：跳过（编码问题太复杂）
            continue;
        }
    }

    inFile.close();

    // 反转记录，使最新的在前面
    std::reverse(records.begin(), records.end());

    // 查找最高分
    FindBestRecord();

    // 初始过滤
    FilterRecords();
}

std::wstring HistoryScene::ParseModeFromString(const std::string &modeStr)
{
    // 旧方法：保留用于兼容（但可能有编码问题）
    return ParseModeFromEnglish(modeStr);
}

std::wstring HistoryScene::ParseModeFromEnglish(const std::string &modeStr)
{
    // 英文模式映射到中文显示
    if (modeStr == "SINGLE" || modeStr == "BEGINNER" ||
        modeStr == "ADVANCED" || modeStr == "EXPERT")
    {
        return L"单人";
    }
    else if (modeStr == "PVP" || modeStr == "LOCAL_PVP" ||
             modeStr == "NET_PVP" || modeStr == "PVE")
    {
        return L"双人";
    }

    // 默认返回
    std::wstring result(modeStr.begin(), modeStr.end());
    return result;
}

void HistoryScene::FilterRecords()
{
    filteredRecords.clear();

    if (currentFilter == 0) // 全部
    {
        filteredRecords = records;
    }
    else if (currentFilter == 1) // 单人
    {
        for (const auto &record : records)
        {
            if (record.mode == L"单人")
            {
                filteredRecords.push_back(record);
            }
        }
    }
    else if (currentFilter == 2) // 双人
    {
        for (const auto &record : records)
        {
            if (record.mode == L"双人")
            {
                filteredRecords.push_back(record);
            }
        }
    }

    // 重置显示起始索引
    displayStartIndex = 0;
}

void HistoryScene::FindBestRecord()
{
    if (records.empty())
    {
        bestRecord = HistoryRecord(L"无记录", L"", 0, 0);
        return;
    }

    bestRecord = records[0];
    for (const auto &record : records)
    {
        if (record.score > bestRecord.score)
        {
            bestRecord = record;
        }
    }
}

// ============== 渲染方法 ==============
void HistoryScene::Render()
{
    BeginBatchDraw();
    cleardevice();

    // A区 - 顶部最高分卡片
    DrawBestRecordCard();

    // 模式过滤按钮
    DrawFilterButtons();

    // C区 - 历史记录列表
    DrawRecordsList();

    // D区 - 底部返回按钮
    DrawFooter();

    EndBatchDraw();
}

void HistoryScene::DrawBestRecordCard()
{
    int cardX = 560;
    int cardY = 120;
    int cardWidth = 800;
    int cardHeight = 180;
    int radius = 15;

    // 绘制阴影效果
    setfillcolor(RGB(17, 45, 78));
    setlinecolor(RGB(17, 45, 78));
    solidroundrect(cardX + 5, cardY + 5, cardX + cardWidth + 5, cardY + cardHeight + 5, radius, radius);

    // 绘制卡片背景
    setfillcolor(COLOR_ACCENT);
    setlinecolor(COLOR_ACCENT);
    solidroundrect(cardX, cardY, cardX + cardWidth, cardY + cardHeight, radius, radius);

    // 居中标题
    settextstyle(36, 0, L"微软雅黑");
    settextcolor(COLOR_WHITE);
    setbkmode(TRANSPARENT);
    const wchar_t *title = L"历史最高分";
    int titleWidth = textwidth(title);
    outtextxy(cardX + (cardWidth - titleWidth) / 2, cardY + 30, title);

    // 绘制分数（大字，居中）
    settextstyle(80, 0, L"微软雅黑");
    settextcolor(COLOR_WHITE);
    std::wstring scoreText = std::to_wstring(bestRecord.score);
    int textWidth = textwidth(scoreText.c_str());
    outtextxy(cardX + (cardWidth - textWidth) / 2, cardY + 70, scoreText.c_str());

    // 绘制时间（小字，居中）
    settextstyle(22, 0, L"微软雅黑");
    settextcolor(RGB(220, 230, 240));
    std::wstring timeText = L"创造于 " + bestRecord.time;
    textWidth = textwidth(timeText.c_str());
    outtextxy(cardX + (cardWidth - textWidth) / 2, cardY + cardHeight - 35, timeText.c_str());
}

void HistoryScene::DrawTrophy(int x, int y, int size)
{
    // 简易奖杯图标：黄色实心圆 + 杯身
    COLORREF goldColor = RGB(255, 215, 0); // 金色

    // 杯口（椭圆）
    setfillcolor(goldColor);
    setlinecolor(goldColor);
    solidellipse(x, y, x + size, y + size / 3);

    // 杯身（梯形，用多边形近似）
    POINT points[4];
    points[0] = {x + size / 6, y + size / 3};
    points[1] = {x + size * 5 / 6, y + size / 3};
    points[2] = {x + size * 4 / 5, y + size * 2 / 3};
    points[3] = {x + size / 5, y + size * 2 / 3};
    solidpolygon(points, 4);

    // 杯底（矩形）
    solidrectangle(x + size / 4, y + size * 2 / 3, x + size * 3 / 4, y + size);
}

void HistoryScene::DrawFilterButtons()
{
    int buttonY = 340;
    int buttonWidth = 200;
    int buttonHeight = 50;
    int totalWidth = buttonWidth * 3;
    int startX = (1920 - totalWidth) / 2;
    int radius = 25;

    // 绘制底座（灰蓝色大胶囊）
    setfillcolor(COLOR_BORDER);
    setlinecolor(COLOR_BORDER);
    solidroundrect(startX, buttonY, startX + totalWidth, buttonY + buttonHeight, radius, radius);

    // 绘制选中块（深蓝色滑块）
    int selectedX = startX + currentFilter * buttonWidth;
    setfillcolor(COLOR_DARK);
    setlinecolor(COLOR_DARK);
    solidroundrect(selectedX, buttonY, selectedX + buttonWidth, buttonY + buttonHeight, radius, radius);

    // 绘制文字
    const wchar_t *labels[] = {L"全部", L"单人", L"双人"};
    settextstyle(30, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);

    for (int i = 0; i < 3; i++)
    {
        int x = startX + i * buttonWidth;
        COLORREF textColor = (i == currentFilter) ? COLOR_WHITE : COLOR_DARK;
        settextcolor(textColor);

        int textWidth = textwidth(labels[i]);
        int textHeight = textheight(labels[i]);
        outtextxy(x + (buttonWidth - textWidth) / 2, buttonY + (buttonHeight - textHeight) / 2, labels[i]);
    }
}

void HistoryScene::DrawRecordsList()
{
    int listX = 360;
    int listY = 440;
    int listWidth = 1200;
    int itemHeight = 75; // 增加5px呼吸感
    int spacing = 15;

    // 定义列X坐标（与DrawSingleRecord对齐）
    int modeX = listX + 15;              // 模式标签起始位置
    int timeX = listX + 110;             // 时间起始位置
    int lengthX = listX + 500;           // 长度起始位置
    int scoreX = listX + listWidth - 30; // 得分右对齐位置

    // B区 - 列表表头（严格对齐）
    settextstyle(26, 0, L"微软雅黑");
    settextcolor(RGB(100, 100, 100));
    setbkmode(TRANSPARENT);
    outtextxy(timeX, listY - 35, L"时间");
    outtextxy(modeX + 20, listY - 35, L"模式");
    outtextxy(lengthX, listY - 35, L"长度");
    outtextxy(scoreX - 50, listY - 35, L"得分");

    // C区 - 记录列表
    int displayCount = min((int)filteredRecords.size() - displayStartIndex, MAX_DISPLAY_RECORDS);

    for (int i = 0; i < displayCount; i++)
    {
        int index = displayStartIndex + i;
        if (index >= (int)filteredRecords.size())
            break;

        int y = listY + i * (itemHeight + spacing);
        DrawSingleRecord(filteredRecords[index], listX, y, listWidth, itemHeight);
    }

    // 如果没有记录，显示提示
    if (filteredRecords.empty())
    {
        settextstyle(36, 0, L"微软雅黑");
        settextcolor(RGB(180, 180, 180));
        setbkmode(TRANSPARENT);
        const wchar_t *emptyText = L"暂无记录";
        int textWidth = textwidth(emptyText);
        outtextxy(listX + (listWidth - textWidth) / 2, listY + 100, emptyText);
    }
}

void HistoryScene::DrawSingleRecord(const HistoryRecord &record, int x, int y, int width, int height)
{
    // 绘制背景
    setfillcolor(RGB(255, 255, 255));
    setlinecolor(COLOR_BORDER);
    solidroundrect(x, y, x + width, y + height, 8, 8);

    // 绘制边框
    setlinecolor(COLOR_BORDER);
    roundrect(x, y, x + width, y + height, 8, 8);

    // 绘制模式标签（彩色小块）
    COLORREF modeColor = (record.mode == L"单人") ? COLOR_ACCENT : COLOR_DARK;
    setfillcolor(modeColor);
    setlinecolor(modeColor);
    solidroundrect(x + 15, y + height / 2 - 15, x + 85, y + height / 2 + 15, 6, 6);

    // 模式文字
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(COLOR_WHITE);
    setbkmode(TRANSPARENT);
    int textWidth = textwidth(record.mode.c_str());
    outtextxy(x + 50 - textWidth / 2, y + height / 2 - 12, record.mode.c_str());

    // 时间（简化显示，只显示月-日 时:分）
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(RGB(128, 128, 128)); // 灰色
    std::wstring timeDisplay = record.time.length() >= 16 ? record.time.substr(5, 11) : record.time;
    outtextxy(x + 110, y + height / 2 - 12, timeDisplay.c_str());

    // 长度
    settextstyle(28, 0, L"微软雅黑");
    settextcolor(COLOR_DARK);
    std::wstring lengthText = L"L:" + std::to_wstring(record.length);
    outtextxy(x + 500, y + height / 2 - 14, lengthText.c_str());

    // 得分（右对齐，粗体）
    settextstyle(36, 0, L"微软雅黑");
    settextcolor(COLOR_DARK);
    std::wstring scoreText = std::to_wstring(record.score);
    textWidth = textwidth(scoreText.c_str());
    outtextxy(x + width - textWidth - 30, y + height / 2 - 18, scoreText.c_str());
}

void HistoryScene::DrawFooter()
{
    int footerY = 900;
    int footerHeight = 180;

    // 绘制白色底栏（悬浮感）
    setfillcolor(RGB(249, 247, 247));
    setlinecolor(RGB(249, 247, 247));
    solidrectangle(0, footerY, 1920, 1080);

    // 翻页按钮
    int pageButtonWidth = 120;
    int pageButtonHeight = 60;
    int pageButtonY = footerY + 20;

    int totalRecords = (int)filteredRecords.size();
    int totalPages = (totalRecords + MAX_DISPLAY_RECORDS - 1) / MAX_DISPLAY_RECORDS;
    int currentPage = displayStartIndex / MAX_DISPLAY_RECORDS + 1;

    // 上一页按钮
    int prevX = 400;
    bool hasPrev = displayStartIndex > 0;
    COLORREF prevColor = hasPrev ? COLOR_ACCENT : COLOR_BORDER;
    setfillcolor(prevColor);
    setlinecolor(prevColor);
    solidroundrect(prevX, pageButtonY, prevX + pageButtonWidth, pageButtonY + pageButtonHeight, 10, 10);

    settextstyle(28, 0, L"微软雅黑");
    settextcolor(hasPrev ? COLOR_WHITE : RGB(150, 150, 150));
    setbkmode(TRANSPARENT);
    const wchar_t *prevText = L"上一页";
    int tw = textwidth(prevText);
    outtextxy(prevX + (pageButtonWidth - tw) / 2, pageButtonY + 16, prevText);

    // 页码显示
    std::wstring pageInfo = L"第 " + std::to_wstring(currentPage) + L" / " + std::to_wstring(totalPages) + L" 页";
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(COLOR_DARK);
    tw = textwidth(pageInfo.c_str());
    outtextxy((1920 - tw) / 2, pageButtonY + 18, pageInfo.c_str());

    // 下一页按钮
    int nextX = 1400;
    bool hasNext = displayStartIndex + MAX_DISPLAY_RECORDS < totalRecords;
    COLORREF nextColor = hasNext ? COLOR_ACCENT : COLOR_BORDER;
    setfillcolor(nextColor);
    setlinecolor(nextColor);
    solidroundrect(nextX, pageButtonY, nextX + pageButtonWidth, pageButtonY + pageButtonHeight, 10, 10);

    settextstyle(28, 0, L"微软雅黑");
    settextcolor(hasNext ? COLOR_WHITE : RGB(150, 150, 150));
    const wchar_t *nextText = L"下一页";
    tw = textwidth(nextText);
    outtextxy(nextX + (pageButtonWidth - tw) / 2, pageButtonY + 16, nextText);

    // 返回按钮
    int returnX = 760;
    int returnY = footerY + 100;
    int returnWidth = 400;
    int returnHeight = 70;

    setfillcolor(COLOR_ACCENT);
    setlinecolor(COLOR_ACCENT);
    solidroundrect(returnX, returnY, returnX + returnWidth, returnY + returnHeight, 12, 12);

    settextstyle(36, 0, L"微软雅黑");
    settextcolor(COLOR_WHITE);
    setbkmode(TRANSPARENT);
    const wchar_t *text = L"返回主菜单";
    int textWidth = textwidth(text);
    int textHeight = textheight(text);
    outtextxy(returnX + (returnWidth - textWidth) / 2, returnY + (returnHeight - textHeight) / 2, text);
}

void HistoryScene::DrawRoundRect(int x, int y, int width, int height, int radius, COLORREF fillColor, COLORREF borderColor)
{
    setfillcolor(fillColor);
    setlinecolor(borderColor);
    solidroundrect(x, y, x + width, y + height, radius, radius);
}

// ============== 输入处理 ==============
void HistoryScene::HandleInput()
{
    MOUSEMSG msg;
    while (inputMgr.GetNextMouseMessage(msg))
    {
        if (msg.uMsg == WM_LBUTTONDOWN)
        {
            int mouseX = msg.x;
            int mouseY = msg.y;

            // 检测过滤按钮点击（胶囊式布局）
            int buttonY = 340;
            int buttonWidth = 200;
            int buttonHeight = 50;
            int totalWidth = buttonWidth * 3;
            int startX = (1920 - totalWidth) / 2;

            for (int i = 0; i < 3; i++)
            {
                int x = startX + i * buttonWidth;
                if (IsMouseInRect(mouseX, mouseY, x, buttonY, buttonWidth, buttonHeight))
                {
                    currentFilter = i;
                    displayStartIndex = 0; // 重置分页
                    FilterRecords();
                    break;
                }
            }

            // 翻页按钮
            int pageButtonWidth = 120;
            int pageButtonHeight = 60;
            int pageButtonY = 920;
            int prevX = 400;
            int nextX = 1400;

            // 上一页按钮
            if (IsMouseInRect(mouseX, mouseY, prevX, pageButtonY, pageButtonWidth, pageButtonHeight))
            {
                if (displayStartIndex > 0)
                {
                    displayStartIndex -= MAX_DISPLAY_RECORDS;
                    if (displayStartIndex < 0)
                        displayStartIndex = 0;
                }
            }

            // 下一页按钮
            if (IsMouseInRect(mouseX, mouseY, nextX, pageButtonY, pageButtonWidth, pageButtonHeight))
            {
                if (displayStartIndex + MAX_DISPLAY_RECORDS < (int)filteredRecords.size())
                {
                    displayStartIndex += MAX_DISPLAY_RECORDS;
                }
            }

            // 检测返回按钮点击
            int returnX = 760;
            int returnY = 1000;
            int returnWidth = 400;
            int returnHeight = 70;
            if (IsMouseInRect(mouseX, mouseY, returnX, returnY, returnWidth, returnHeight))
            {
                sceneRunning = false;
            }
        }
    }

    // ESC 键返回
    if (inputMgr.IsKeyJustPressed(VK_ESCAPE))
    {
        sceneRunning = false;
    }

    // 上下翻页（如果记录超过显示数量）
    if (inputMgr.IsKeyJustPressed(VK_UP))
    {
        if (displayStartIndex > 0)
        {
            displayStartIndex -= MAX_DISPLAY_RECORDS;
            if (displayStartIndex < 0)
                displayStartIndex = 0;
        }
    }

    if (inputMgr.IsKeyJustPressed(VK_DOWN))
    {
        if (displayStartIndex + MAX_DISPLAY_RECORDS < (int)filteredRecords.size())
        {
            displayStartIndex += MAX_DISPLAY_RECORDS;
        }
    }
}

bool HistoryScene::IsMouseInRect(int mouseX, int mouseY, int x, int y, int width, int height)
{
    return mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height;
}
