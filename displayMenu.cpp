#include "mat.h"
#include <iostream>
#include <conio.h>
using namespace std;

// 修改：exitFunction 返回 bool
bool exitFunction() {
    cout << "\n 确定退出当前操作并返回菜单吗? (y/n): ";
    char ch = _getch(); // 不需要按回车即可输入
    cout << ch << endl; // 显示用户输入的字符
    if (ch == 'y' || ch == 'Y') {
        cout << "返回菜单界面..." << endl;
        return false; // 退出当前操作
    } else {
        cout << "继续当前操作..." << endl;
        return true; // 继续当前操作
    }
}
// 新增：按任意键返回主菜单的函数
void waitAndReturnToMenu() {
    cout << "\n按任意键返回主菜单..." << endl;
    _getch();
}

// 打包清屏和退出判断逻辑的函数
bool pre(const string& operation) {
    system("cls"); // 清屏函数
    cout << "执行" << operation << "操作" << endl;
    cout << "输入0可退出当前操作，输入其他任意字符继续: ";
    char input = _getch();
    cout << input << endl;
    if (input == '0') {
        return exitFunction(); // 直接返回 exitFunction 的结果
    }
    return true; // 返回 true 表示继续操作
}

void matriplus() {//未检查输入行列数是否合法
    if (!pre("矩阵加法")) return; // 调用 pre 函数
    system("cls");
    cout << "正在进行矩阵加法操作" << endl;
    int rows, cols;
    cout << "请输入矩阵的行数: ";
    cin >> rows;
    cout << "请输入矩阵的列数: ";
    cin >> cols;

    Matrix m1(rows, cols), m2(rows, cols);

    cout << "请输入第一个矩阵:" << endl;
    m1.input();

    cout << "请输入第二个矩阵:" << endl;
    m2.input();

    try {
        Matrix result = m1 + m2;
        cout << "矩阵加法结果为:" << endl;
        result.print();
        waitAndReturnToMenu(); // 新增：等待用户按键并返回主菜单
    } catch (const std::exception& e) {
        cout << "发生错误: " << e.what() << endl;
    }
}

void nummulti() {
    if (!pre("数乘")) return; // 调用 pre 函数
    system("cls");
    cout << "正在进行数乘操作" << endl;
    int rows, cols;
    cout << "请输入矩阵的行数: ";
    cin >> rows;
    cout << "请输入矩阵的列数: ";
    cin >> cols;

    Matrix m(rows, cols);
    cout << "请输入矩阵元素:" << endl;
    m.input();

    int num;
    cout << "请输入要乘的数: ";
    cin >> num;

    try {
        Matrix result = m * num;
        cout << "数乘结果为:" << endl;
        result.print();
        waitAndReturnToMenu();
    } catch (const std::exception& e) {
        cout << "发生错误: " << e.what() << endl;
    }
}

void matritrans() {
    if (!pre("矩阵转置")) return; // 调用 pre 函数
    system("cls");
    cout << "正在进行矩阵转置操作" << endl;
    int rows, cols;
    cout << "请输入矩阵的行数: ";
    cin >> rows;
    cout << "请输入矩阵的列数: ";
    cin >> cols;

    Matrix m(rows, cols);
    cout << "请输入矩阵元素:" << endl;
    m.input();

    try {
        Matrix result = m.transpose();
        cout << "转置结果为:" << endl;
        result.print();
        waitAndReturnToMenu();
    } catch (const std::exception& e) {
        cout << "发生错误: " << e.what() << endl;
    }
}

void matrimulti() {
    if (!pre("矩阵乘法")) return; // 调用 pre 函数
    system("cls");
    cout << "正在进行矩阵乘法操作" << endl;
    int rows1, cols1, rows2, cols2;
    cout << "请输入第一个矩阵的行数: ";
    cin >> rows1;
    cout << "请输入第一个矩阵的列数: ";
    cin >> cols1;
    cout << "请输入第二个矩阵的行数: ";
    cin >> rows2;
    cout << "请输入第二个矩阵的列数: ";
    cin >> cols2;

    if (cols1 != rows2) {
        cout << "矩阵维度不匹配，无法相乘。" << endl;
        waitAndReturnToMenu();
        return;
    }

    Matrix m1(rows1, cols1), m2(rows2, cols2);

    cout << "请输入第一个矩阵元素:" << endl;
    m1.input();

    cout << "请输入第二个矩阵元素:" << endl;
    m2.input();

    try {
        Matrix result = m1 * m2;
        cout << "矩阵乘法结果为:" << endl;
        result.print();
        waitAndReturnToMenu();
    } catch (const std::exception& e) {
        cout << "发生错误: " << e.what() << endl;
    }
}

void hadamulti() {
    if (!pre("Hadamard乘积")) return; // 调用 pre 函数
    system("cls");
    cout << "正在进行Hadamard乘积操作" << endl;

    int rows, cols;
    cout << "请输入矩阵的行数: ";
    cin >> rows;
    cout << "请输入矩阵的列数: ";
    cin >> cols;

    Matrix m1(rows, cols), m2(rows, cols);

    cout << "请输入第一个矩阵元素:" << endl;
    m1.input();

    cout << "请输入第二个矩阵元素:" << endl;
    m2.input();

    try {
        Matrix result = m1.hadamard(m2);
        cout << "Hadamard乘积结果为:" << endl;
        result.print();
        waitAndReturnToMenu();
    } catch (const std::exception& e) {
        cout << "发生错误: " << e.what() << endl;
    }
}

void conv() {
    if (!pre("卷积")) return; // 调用 pre 函数
    system("cls");
    cout << "正在进行卷积操作" << endl;

    int rows, cols;
    cout << "请输入待卷积矩阵的行数: ";
    cin >> rows;
    cout << "请输入待卷积矩阵的列数: ";
    cin >> cols;

    Matrix m(rows, cols);
    cout << "请输入待卷积矩阵元素:" << endl;
    m.input();

    cout << "请输入3x3卷积核矩阵元素:" << endl;
    Matrix kernel(3, 3);
    kernel.input();

    try {
        Matrix result = m.convolve3x3(kernel);
        cout << "卷积结果为:" << endl;
        result.print();
        waitAndReturnToMenu();
    } catch (const std::exception& e) {
        cout << "发生错误: " << e.what() << endl;
    }
}

//不要对下面的函数做任何调整
void menu() {
    cout << "菜单选项：" << endl;
    cout << "1. 矩阵加法" << endl;
    cout << "2. 数乘" << endl;
    cout << "3. 矩阵转置" << endl;
    cout << "4. 矩阵乘法" << endl;
    cout << "5. Hadamard乘积" << endl;
    cout << "6. 卷积操作" << endl;
    cout << "7. 示例演示" << endl;
    cout << "0. 退出程序" << endl;
}
