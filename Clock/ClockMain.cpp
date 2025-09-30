#include <graphics.h>   // EasyX 图形库
#include <conio.h>      // _kbhit(), _getch()
#include <ctime>        // time(), localtime_s()
#include <cmath>        // sin(), cos() 用于计算指针坐标
#include <Windows.h>    // Sleep() 函数

// 前置声明，具体实现可以后续添加
void init();            // 初始化时钟界面
void drawClock();       // 绘制时钟表盘
void drawHands(int hour, int minute, int second);  // 绘制时分秒针

int main()
{
    struct tm t;
    time_t now;
    time(&now);
    localtime_s(&t, &now);	// 获取当地时间
    initgraph(640, 480);	// 图形方式初始化
    init();			// 自定义图形初始化函数，用于绘制时钟界面

    while (!_kbhit())	// 无键盘操作时进入循环
    {
        /* 不停获取当前时间，与之前绘制时/分/秒针时的时间相比，
        以判断新时间的到来 */
        time(&now);
        localtime_s(&t, &now);
        
        /* 根据新的时间重新绘制时针，分针，秒针，
        可以自定义函数完成，注意坐标数据类型 */
        drawHands(t.tm_hour, t.tm_min, t.tm_sec);
        
        Sleep(100);  // 避免过度占用CPU
    }
    _getch();		// 按任意键准备退出时钟程序
    closegraph();		// 退出图形界面
    return 0;
}