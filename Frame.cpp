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

void demo()
{
    // /* 对vs+opencv正确配置后方可使用，此处只给出一段读取并显示图像的参考代码，其余功能流程自行设计和查阅文献 */
    // Mat image =
    //     imread("demolena.jpg"); // 图像的灰度值存放在格式为Mat的变量image中
    // imshow("Image-original", image);
    // waitKey(0);

    // 提示：Mat格式可与数组相互转换
    cout<<"演示函数" << endl;
    return;
}

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
            char ch;
            cout << "\n 确定退出吗?确定请输入y" << endl;
            cin >> ch;
            if (ch == 'y' || ch == 'Y'){
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