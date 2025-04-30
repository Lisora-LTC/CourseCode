#ifndef MAT_H
#define MAT_H

#include <vector>
#include <iostream>

// 定义矩阵类
class Matrix {
private:
    std::vector<int> data; // 使用一维数组存储矩阵数据
    int rows; // 行数
    int cols; // 列数

public:
    // 构造函数
    Matrix(int rows, int cols);
    Matrix(int rows, int cols, const int* values, int length);

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

    // 打印矩阵
    void print() const;

    // 输入矩阵元素
    void input();
};

// 声明矩阵操作相关函数
void menu();        // 声明菜单显示函数
void matriplus();   // 矩阵加法
void nummulti();    // 数乘
void matritrans();  // 矩阵转置
void matrimulti();  // 矩阵乘法
void hadamulti();   // Hadamard乘积
void conv();        // 卷积操作
void waitAndReturnToMenu();
bool exitFunction(); // 声明退出函数

#endif // MAT_H
