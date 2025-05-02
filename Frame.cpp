#include <conio.h>
#include <iostream>
#include "mat.h" // 包含矩阵操作头文件
//#include <opencv2/opencv.hpp>
//using namespace cv;
using namespace std;

// 此框架若有不完美可以在作业中任意修改

void wait_for_enter()
{
    cout << endl
         << "按回车键继续";
    while (_getch() != '\r')
        ;
    cout << endl
         << endl;
}

// demo 函数已移动到 displayMenu.cpp 中

int main()
{
    // 定义相关变量

    wait_for_enter();
    while (true) // 注意该循环退出的条件
    {
        system("cls"); // 清屏函数

        menu(); // 调用菜单显示函数，自行补充完成

        // 按要求输入菜单选择项choice
        char choice;
        cout << "请输入菜单选择项 (0-7): ";
        choice = _getch(); // 不需要按回车即可输入
        cout << choice << endl; // 显示用户输入的字符
        if (choice == '0') // 选择退出
        {
            cout << "\n 确定退出吗?按回车键确认退出，按其他键取消" << endl;
            if (_getch() == '\r'){ // 检测是否按下回车键
                break;
            }else{
                continue;
            }
        }
        switch (choice)
        {
        // 下述矩阵操作函数自行设计并完成（包括函数参数及返回类型等），若选择加分项，请自行补充
        case '1':
            matriplus();
            break;
        case '2':
            nummulti();
            break;
        case '3':
            matritrans();
            break;
        case '4':
            matrimulti();
            break;
        case '5':
            hadamulti();
            break;
        case '6':
            conv();
            break;
        case '7':
            demo();
            break;
        default:
            cout << "\n 输入错误，请从新输入" << endl;
            wait_for_enter();
        }
    }
    return 0;
}