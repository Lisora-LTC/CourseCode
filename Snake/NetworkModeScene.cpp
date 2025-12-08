#include <winsock2.h>
#include "NetworkModeScene.h"
#include <windows.h>

// ============== 构造与析构 ==============
NetworkModeScene::NetworkModeScene(bool manageWindow)
    : sceneRunning(false), manageWindow(manageWindow), selectedAction(NM_NONE)
{
    InitUI();
}

NetworkModeScene::~NetworkModeScene()
{
    if (manageWindow)
    {
        closegraph();
    }
}

// ============== 主方法 ==============
NetworkModeAction NetworkModeScene::Show()
{
    if (manageWindow)
    {
        initgraph(1920, 1080);
    }

    setbkcolor(COLOR_BG);
    cleardevice();

    sceneRunning = true;
    selectedAction = NM_NONE;

    while (sceneRunning)
    {
        if (inputMgr.IsWindowClosed())
        {
            // 窗口关闭时正常退出，让上层调用者处理清理
            selectedAction = NM_BACK;
            sceneRunning = false;
            break;
        }

        inputMgr.Update();
        HandleInput();
        Render();
        Sleep(10);
    }

    return selectedAction;
}

// ============== 初始化 ==============
void NetworkModeScene::InitUI()
{
    // 计算按钮位置（与 MenuScene 完全一致）
    int buttonWidth = 600;  // 与首页一致
    int buttonHeight = 100; // 与首页一致
    int centerX = 960;      // 屏幕中心X
    int startX = 660;       // 1920/2 - 300
    int startY = 360;       // 与首页子菜单一致
    int spacing = 160;      // 与首页主菜单一致

    // 创建房间按钮（胶囊形、无阴影）
    btnCreateRoom = std::make_unique<UIButton>(
        startX,
        startY,
        buttonWidth,
        buttonHeight,
        L"创建房间",
        BUTTON_CAPSULE);
    btnCreateRoom->SetColors(RGB(219, 226, 239), RGB(85, 132, 188), RGB(63, 114, 175));
    btnCreateRoom->SetTextColor(RGB(17, 45, 78), RGB(255, 255, 255));
    btnCreateRoom->SetFontSize(48);

    // 加入房间按钮（胶囊形、无阴影）
    btnJoinRoom = std::make_unique<UIButton>(
        startX,
        startY + spacing,
        buttonWidth,
        buttonHeight,
        L"加入房间",
        BUTTON_CAPSULE);
    btnJoinRoom->SetColors(RGB(219, 226, 239), RGB(85, 132, 188), RGB(63, 114, 175));
    btnJoinRoom->SetTextColor(RGB(17, 45, 78), RGB(255, 255, 255));
    btnJoinRoom->SetFontSize(48);

    // 返回按钮（胶囊形、无阴影）
    btnBack = std::make_unique<UIButton>(
        startX,
        startY + spacing * 2,
        buttonWidth,
        buttonHeight,
        L"返回主菜单",
        BUTTON_CAPSULE);
    btnBack->SetColors(RGB(219, 226, 239), RGB(85, 132, 188), RGB(63, 114, 175));
    btnBack->SetTextColor(RGB(17, 45, 78), RGB(255, 255, 255));
    btnBack->SetFontSize(48);
}

// ============== 渲染方法 ==============
void NetworkModeScene::Render()
{
    BeginBatchDraw();

    setbkcolor(COLOR_BG);
    cleardevice();

    DrawTitle();
    DrawButtons();

    EndBatchDraw();
}

void NetworkModeScene::DrawTitle()
{
    // 绘制大标题
    settextcolor(COLOR_DARK);
    settextstyle(72, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);

    const wchar_t *title = L"联机对战";
    int titleWidth = textwidth(title);
    int titleHeight = textheight(title);

    outtextxy(960 - titleWidth / 2, 200, title);

    // 绘制副标题
    settextcolor(RGB(100, 100, 100));
    settextstyle(32, 0, L"微软雅黑");

    const wchar_t *subtitle = L"选择游戏模式";
    int subtitleWidth = textwidth(subtitle);

    outtextxy(960 - subtitleWidth / 2, 290, subtitle);
}

void NetworkModeScene::DrawButtons()
{
    // 更新按钮状态
    int mouseX, mouseY;
    inputMgr.GetMousePosition(mouseX, mouseY);
    bool mousePressed = GetAsyncKeyState(VK_LBUTTON) & 0x8000;

    btnCreateRoom->UpdateState(mouseX, mouseY, mousePressed);
    btnJoinRoom->UpdateState(mouseX, mouseY, mousePressed);
    btnBack->UpdateState(mouseX, mouseY, mousePressed);

    // 绘制按钮
    btnCreateRoom->Draw();
    btnJoinRoom->Draw();
    btnBack->Draw();
}

// ============== 输入处理 ==============
void NetworkModeScene::HandleInput()
{
    if (inputMgr.IsLeftButtonJustPressed())
    {
        int mouseX, mouseY;
        inputMgr.GetMousePosition(mouseX, mouseY);

        // 检测按钮点击
        if (btnCreateRoom->IsMouseOver(mouseX, mouseY))
        {
            selectedAction = NM_CREATE_ROOM;
            sceneRunning = false;
        }
        else if (btnJoinRoom->IsMouseOver(mouseX, mouseY))
        {
            selectedAction = NM_JOIN_ROOM;
            sceneRunning = false;
        }
        else if (btnBack->IsMouseOver(mouseX, mouseY))
        {
            selectedAction = NM_BACK;
            sceneRunning = false;
        }
    }

    // ESC键返回
    if (inputMgr.IsKeyJustPressed(VK_ESCAPE))
    {
        selectedAction = NM_BACK;
        sceneRunning = false;
    }
}
