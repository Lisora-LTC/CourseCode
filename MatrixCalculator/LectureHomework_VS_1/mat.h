#ifndef MAT_H
#define MAT_H

#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>

// 定义矩阵类
class Matrix {
private:
    int* data; // 使用原始指针存储矩阵数据
    int rows; // 行数
    int cols; // 列数

public:
    // 构造函数
    Matrix(int rows, int cols);
    Matrix(const Matrix& other); // 拷贝构造函数
    Matrix& operator=(const Matrix& other); //  Z
    ~Matrix(); // 析构函数

    // 获取矩阵的行数和列数
    int getRows() const;
    int getCols() const;

    // 获取和设置元素的方法
    int get(int i, int j) const; // 获取(i,j)位置的元素
    void set(int i, int j, int value); // 设置(i,j)位置的元素

    // 矩阵加法
    Matrix operator+(const Matrix& other) const;

    // 矩阵乘法
    Matrix operator*(const Matrix& other) const;

    // 数乘
    Matrix operator*(int num) const;

    // 转置
    Matrix transpose() const;

    // Hadamard 乘积
    Matrix hadamard(const Matrix& other) const;

    // 3x3卷积，kernel size=3, padding=1, stride=1, dilation=1，需要传入3x3核
    Matrix convolve3x3(const Matrix& kernel) const;

    // 矩阵归一化 - 将所有元素除以指定因子
    void normalize(int factor);

    // 将矩阵转换为OpenCV灰度图像
    cv::Mat toGrayImage() const;

    // 打印矩阵
    void print() const;

    // 输入矩阵元素
    void input();

    // Otsu算法：对当前矩阵进行二值化，返回二值化后的矩阵
    Matrix otsuThreshold() const;
};

// 声明矩阵操作相关函数
void demo();        // demo 函数声明
void wait_for_enter(); // 确保 wait_for_enter 函数也有声明
void menu();        // 声明菜单显示函数
void matriplus();   // 矩阵加法
void nummulti();    // 数乘
void matritrans();  // 矩阵转置
void matrimulti();  // 矩阵乘法
void hadamulti();   // Hadamard乘积
void conv();        // 卷积操作
void waitAndReturnToMenu();
bool exitFunction(); // 声明退出函数
void inputErrorReturn();
void otsu(); // otsu算法函数声明

#endif // MAT_H
