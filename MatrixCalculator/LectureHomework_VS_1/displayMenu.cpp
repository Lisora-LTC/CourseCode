#include "mat.h"
#include <iostream>
#include <conio.h>
#include <opencv2/opencv.hpp>

using namespace cv;
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
    if (!pre("演示")) return; // 调用 pre 函数，可以选择退出
    system("cls"); // 清屏
    cout << "正在进行演示操作" << endl;

    Mat image = imread("demolena.jpg", IMREAD_GRAYSCALE); // 直接以灰度模式读取图像
    if (image.empty()) {
        cout << "无法打开图像文件！" << endl;
        waitAndReturnToMenu();
        return;
    }
    imshow("Image-original", image);
    moveWindow("Image-original", 100, 100); // 原图像在最左上角独自成列

    int rows = image.rows;
    int cols = image.cols;
    cout << "图像尺寸: " << rows << "×" << cols << " 像素" << endl;

    Matrix imageMatrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int pixelValue = static_cast<int>(image.at<uchar>(i, j));
            imageMatrix.set(i, j, pixelValue);
        }
    }
    cout << "已将图像灰度值存入Matrix对象" << endl;

    // 用6*9的数组存储6个3x3卷积核
    int kernels[6][9] = {
        { 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { -1, -2, -1, 0, 0, 0, 1, 2, 1 },
        { -1, 0, 1, -2, 0, 2, -1, 0, 1 },
        { -1, -1, -1, -1, 9, -1, -1, -1, -1 },
        { -1, -1, 0, -1, 0, 1, 0, 1, 1 },
        { 1, 2, 1, 2, 4, 2, 1, 2, 1 }
    };
    const char* kernelNames[6] = {
        "全1核", "Sobel Y", "Sobel X", "锐化", "自定义", "高斯模糊"
    };

    for (int k = 0; k < 6; ++k) {
        cout << "\n正在使用卷积核: " << kernelNames[k] << endl;
        Matrix kernel(3, 3);
        // 按顺序赋值
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                kernel.set(i, j, kernels[k][i * 3 + j]);
        kernel.print();

        Matrix convResult = imageMatrix.convolve3x3(kernel);

        // 归一化因子
        int normDiv = 1;
        if (k == 0) normDiv = 9; // 全1核
        if (k == 5) normDiv = 16; // 高斯模糊
        convResult.normalize(normDiv);

        Mat resultImage = convResult.toGrayImage();
        string winName = string("Conv-") + kernelNames[k];
        imshow(winName, resultImage);

        // 指定窗口位置，原图在最左上角，卷积结果在右面按2行3列排列
        moveWindow(winName, 500 + (k % 3) * 350, 100 + (k / 3) * 350);

        // string saveName = string("convolution_result_") + to_string(k + 1) + ".jpg";
        // imwrite(saveName, resultImage);
        // cout << "已保存: " << saveName << endl;
    }

    cout << "按任意键关闭所有卷积结果窗口..." << endl;
    waitKey();
    destroyAllWindows(); // 关闭所有由 OpenCV 创建的窗口
    cout << "演示函数执行完毕" << endl;
    waitAndReturnToMenu();
    return;
}

void otsu() {
    if (!pre("Otsu算法")) return; // 调用 pre 函数，可以选择退出
    system("cls"); // 清屏
    cout << "正在进行Otsu算法操作" << endl;

    // 图片文件名列表
    vector<string> filenames = {
        "demolena.jpg", "snowball.jpg", "polyhedrosis.jpg", "ship.jpg", "brain.jpg"
    };

    vector<cv::Mat> originals;
    vector<cv::Mat> results;

    for (int idx = 0; idx < filenames.size(); ++idx) {
        string fname = filenames[idx];
        Mat image = imread(fname, IMREAD_GRAYSCALE);
        if (image.empty()) {
            cout << "无法打开图像文件: " << fname << endl;
            originals.push_back(Mat::zeros(100, 100, CV_8UC1)); // 占位
            results.push_back(Mat::zeros(100, 100, CV_8UC1));
            continue;
        }
        originals.push_back(image);

        int rows = image.rows;
        int cols = image.cols;
        Matrix mat(rows, cols);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int pixelValue = static_cast<int>(image.at<uchar>(i, j));
                mat.set(i, j, pixelValue);
            }
        }
        Matrix result = mat.otsuThreshold();
        results.push_back(result.toGrayImage());
    }

    // 控制窗口位置
    int x_offset = 100;
    int y_offset1 = 100;
    int y_offset2 = 150 + originals[0].rows;
    int gap = 20;

    int cur_x1 = x_offset;
    int cur_x2 = x_offset;

    for (int i = 0; i < originals.size(); ++i) {
        string winName1 = "原图" + to_string(i+1);
        imshow(winName1, originals[i]);
        moveWindow(winName1, cur_x1, y_offset1);
        cur_x1 += originals[i].cols + gap;
    }
    for (int i = 0; i < results.size(); ++i) {
        string winName2 = "Otsu结果" + to_string(i+1);
        imshow(winName2, results[i]);
        moveWindow(winName2, cur_x2, y_offset2);
        cur_x2 += results[i].cols + gap;
    }

    cout << "Otsu算法操作完成。" << endl;
    cout << "按任意键关闭所有窗口..." << endl;
    waitKey(0);
    destroyAllWindows();
    waitAndReturnToMenu();
}

void menu() {
    cout << "*****************************************" << endl;
    cout << "*                                       *" << endl;
    cout << "*  1. 矩阵加法        2. 数乘           *" << endl;
    cout << "*                                       *" << endl;
    cout << "*  3. 矩阵转置        4. 矩阵乘法       *" << endl;
    cout << "*                                       *" << endl;
    cout << "*  5. Hadamard乘积    6. 卷积操作       *" << endl;
    cout << "*                                       *" << endl;
    cout << "*  7. 示例演示        8. Otsu算法       *" << endl;
    cout << "*                                       *" << endl;
    cout << "*  0. 退出程序                          *" << endl;
    cout << "*                                       *" << endl;
    cout << "*****************************************" << endl;
}
