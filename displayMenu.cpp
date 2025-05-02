#include "mat.h"
#include <iostream>
#include <conio.h>
//#include <opencv2/opencv.hpp>

//using namespace cv;
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

// 新增：输入错误返回主菜单的函数
void inputErrorReturn() {
    std::cout << "输入错误: 矩阵的行数和列数必须大于0。" << std::endl;
    waitAndReturnToMenu();
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

void matriplus() {
    if (!pre("矩阵加法")) return; // 调用 pre 函数
    system("cls");
    cout << "正在进行矩阵加法操作" << endl;
    int rows, cols;
    cout << "请输入矩阵的行数: ";
    cin >> rows;
    cout << "请输入矩阵的列数: ";
    cin >> cols;

    try {
        Matrix m1(rows, cols), m2(rows, cols);

        cout << "请输入第一个矩阵:" << endl;
        m1.input();

        cout << "请输入第二个矩阵:" << endl;
        m2.input();

        //printf("???");
        Matrix result = m1 + m2;
        cout << "矩阵加法结果为:" << endl;
        result.print();
        waitAndReturnToMenu(); // 新增：等待用户按键并返回主菜单
    } catch (const std::invalid_argument& e) {
        inputErrorReturn();
        return;
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

    try {
        Matrix m(rows, cols);

        cout << "请输入矩阵元素:" << endl;
        m.input();

        int num;
        cout << "请输入要乘的数: ";
        cin >> num;

        Matrix result = m * num;
        cout << "数乘结果为:" << endl;
        result.print();
        waitAndReturnToMenu();
    } catch (const std::invalid_argument& e) {
        inputErrorReturn();
        return;
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

    try {
        Matrix m(rows, cols);

        cout << "请输入矩阵元素:" << endl;
        m.input();

        Matrix result = m.transpose();
        cout << "转置结果为:" << endl;
        result.print();
        waitAndReturnToMenu();
    } catch (const std::invalid_argument& e) {
        inputErrorReturn();
        return;
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

    try {
        Matrix m1(rows1, cols1), m2(rows2, cols2);

        if (cols1 != rows2) {
            cout << "矩阵维度不匹配，无法相乘。" << endl;
            waitAndReturnToMenu();
            return;
        }

        cout << "请输入第一个矩阵元素:" << endl;
        m1.input();

        cout << "请输入第二个矩阵元素:" << endl;
        m2.input();

        Matrix result = m1 * m2;
        cout << "矩阵乘法结果为:" << endl;
        result.print();
        waitAndReturnToMenu();
    } catch (const std::invalid_argument& e) {
        inputErrorReturn();
        return;
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

    try {
        Matrix m1(rows, cols), m2(rows, cols);

        cout << "请输入第一个矩阵元素:" << endl;
        m1.input();

        cout << "请输入第二个矩阵元素:" << endl;
        m2.input();

        Matrix result = m1.hadamard(m2);
        cout << "Hadamard乘积结果为:" << endl;
        result.print();
        waitAndReturnToMenu();
    } catch (const std::invalid_argument& e) {
        inputErrorReturn();
        return;
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

    try {
        Matrix m(rows, cols);

        cout << "请输入待卷积矩阵元素:" << endl;
        m.input();

        cout << "请输入3x3卷积核矩阵元素:" << endl;
        Matrix kernel(3, 3);
        kernel.input();

        Matrix result = m.convolve3x3(kernel);
        cout << "卷积结果为:" << endl;
        result.print();
        waitAndReturnToMenu();
    } catch (const std::invalid_argument& e) {
        inputErrorReturn();
        return;
    } catch (const std::exception& e) {
        cout << "发生错误: " << e.what() << endl;
    }
}

void demo()
{
    // if (!pre("演示")) return; // 调用 pre 函数，可以选择退出
    // system("cls"); // 清屏
    // cout << "正在进行演示操作" << endl;
    
    // /* 对vs+opencv正确配置后方可使用，此处只给出一段读取并显示图像的参考代码，其余功能流程自行设计和查阅文献 */
    // Mat image =
    //     imread("demolena.jpg", IMREAD_GRAYSCALE); // 直接以灰度模式读取图像
    // if (image.empty()) {
    //     cout << "无法打开图像文件！" << endl;
    //     waitAndReturnToMenu();
    //     return;
    // }
    // imshow("Image-original", image);
    // cout << "按任意键关闭图像窗口..." << endl;
    // waitKey(); // 等待用户按键
    
    // // 将灰度图像数据转存到Matrix类中
    // int rows = image.rows;
    // int cols = image.cols;
    // cout << "图像尺寸: " << rows << "×" << cols << " 像素" << endl;
    
    // // 创建与图像尺寸相同的Matrix对象
    // Matrix imageMatrix(rows, cols);
    
    // // 将灰度值从Mat复制到Matrix
    // for (int i = 0; i < rows; i++) {
    //     for (int j = 0; j < cols; j++) {
    //         // 获取像素灰度值(0-255)
    //         int pixelValue = static_cast<int>(image.at<uchar>(i, j));
    //         // 设置到我们的Matrix对象中
    //         imageMatrix.set(i, j, pixelValue);
    //     }
    // }
    
    // cout << "已将图像灰度值存入Matrix对象" << endl;
    
    // // 创建3x3的全1卷积核
    // cout << "创建3x3的全1卷积核..." << endl;
    // Matrix kernel(3, 3);
    // for (int i = 0; i < 3; i++) {
    //     for (int j = 0; j < 3; j++) {
    //         kernel.set(i, j, 1);
    //     }
    // }
    
    // cout << "卷积核矩阵:" << endl;
    // kernel.print();
    
    // // 对图像矩阵执行卷积操作
    // cout << "执行卷积操作..." << endl;
    // Matrix convResult = imageMatrix.convolve3x3(kernel);
    
    // // 使用新方法将卷积结果归一化(除以9)
    // cout << "将卷积结果归一化(除以9)..." << endl;
    // convResult.normalize(9);
    
    // // 使用新方法将矩阵转换为灰度图像
    // Mat resultImage = convResult.toGrayImage();
    
    // // 显示卷积后的图像
    // imshow("Image-after-convolution", resultImage);
    // cout << "按任意键关闭卷积结果窗口..." << endl;
    // waitKey();
    
    // // 保存卷积后的图像
    // imwrite("convolution_result.jpg", resultImage);
    // cout << "已将卷积结果保存为convolution_result.jpg" << endl;

    // destroyAllWindows(); // 关闭所有由 OpenCV 创建的窗口

    cout<<"演示函数执行完毕" << endl; // 修改提示信息
    waitAndReturnToMenu(); // 等待用户按任意键返回主菜单
    return;
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
