#include <iostream>
#include <conio.h> // 用于 getch()

using namespace std;

void displayMenu() {
    cout << "主菜单:\n";
    cout << "1. 功能1\n";
    cout << "2. 功能2\n";
    cout << "3. 功能3\n";
    cout << "4. 功能4\n";
    cout << "5. 功能5\n";
    cout << "6. 功能6\n";
    cout << "7. 功能7\n";
    cout << "按对应数字选择操作，按 0 退出。\n";
}

void displayFeatureScreen(int featureNumber) {
    int input = -1;
    while (input != 0) {
        system("cls"); // 清屏
        cout << "功能" << featureNumber << "界面\n";
        cout << "这里可以显示一些内容...\n";
        cout << "输入 0 返回主菜单。\n";
        input = getch() - '0'; // 捕获单个按键并转换为数字
        if (input != 0) {
            cout << "无效输入，请重试。\n";
            getch(); // 等待用户按键
        }
    }
}

void handleOption(int option) {
    switch (option) {
        case 7:
            displayFeatureScreen(option);
            break;
        case 0:
            cout << "退出程序\n";
            break;
        default:
            cout << "无效输入，请重试。\n";
            break;
    }
    return;
}

int main() {
    int option = -1;
    while (option != 0) {
        system("cls"); // 清屏
        displayMenu();
        
        char input = getch(); // 捕获单个按键

        option = input - '0'; // 将字符转换为数字
        handleOption(option);
    }
    cout<<option;
    return 0;
}
