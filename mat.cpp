#include "mat.h"
#include <stdexcept>

// 构造函数
Matrix::Matrix(int rows, int cols) : rows(rows), cols(cols), data(rows * cols, 0) {}

// 新的构造函数，接收一维数组指针和长度
Matrix::Matrix(int rows, int cols, const int* values, int length) : rows(rows), cols(cols), data(rows * cols, 0) {
    if (length != rows * cols) {
        throw std::invalid_argument("输入数据长度与矩阵尺寸不匹配");
    }
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            set(i, j, values[i * cols + j]);
        }
    }
}

// 获取矩阵的行数和列数
int Matrix::getRows() const { return rows; }
int Matrix::getCols() const { return cols; }

// 获取和设置元素
int Matrix::get(int i, int j) const {
    if (i < 0 || i >= rows || j < 0 || j >= cols) {
        throw std::out_of_range("矩阵索引越界");
    }
    return data[i * cols + j];
}

void Matrix::set(int i, int j, int value) {
    if (i < 0 || i >= rows || j < 0 || j >= cols) {
        throw std::out_of_range("矩阵索引越界");
    }
    data[i * cols + j] = value;
}

// 矩阵加法
Matrix Matrix::operator+(const Matrix& other) const {
    if (rows != other.rows || cols != other.cols) {
        throw std::invalid_argument("矩阵维度不匹配，无法相加");
    }
    Matrix result(rows, cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result.set(i, j, get(i, j) + other.get(i, j));
        }
    }
    return result;
}

// 数乘
Matrix Matrix::operator*(int num) const {
    Matrix result(rows, cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result.set(i, j, get(i, j) * num);
        }
    }
    return result;
}

// 转置
Matrix Matrix::transpose() const {
    Matrix result(cols, rows);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result.set(j, i, get(i, j));
        }
    }
    return result;
}

// 矩阵乘法
Matrix Matrix::operator*(const Matrix& other) const {
    if (cols != other.rows) {
        throw std::invalid_argument("矩阵维度不匹配，无法相乘");
    }
    Matrix result(rows, other.cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < other.cols; ++j) {
            int sum = 0;
            for (int k = 0; k < cols; ++k) {
                sum += get(i, k) * other.get(k, j);
            }
            result.set(i, j, sum);
        }
    }
    return result;
}

// Hadamard 乘积
Matrix Matrix::hadamard(const Matrix& other) const {
    if (rows != other.rows || cols != other.cols) {
        throw std::invalid_argument("矩阵维度不匹配，无法进行Hadamard乘积");
    }
    Matrix result(rows, cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result.set(i, j, get(i, j) * other.get(i, j));
        }
    }
    return result;
}

// 3x3卷积，kernel size=3, padding=1, stride=1, dilation=1，需要传入3x3核
Matrix Matrix::convolve3x3(const Matrix& kernel) const {
    if (kernel.getRows() != 3 || kernel.getCols() != 3) {
        throw std::invalid_argument("卷积核必须为3x3矩阵");
    }
    int ksize = 3;
    int pad = 1;
    int stride = 1;
    int dilation = 1;

    int out_rows = (rows + 2 * pad - dilation * (ksize - 1) - 1) / stride + 1;
    int out_cols = (cols + 2 * pad - dilation * (ksize - 1) - 1) / stride + 1;
    Matrix result(out_rows, out_cols);

    for (int i = 0; i < out_rows; ++i) {
        for (int j = 0; j < out_cols; ++j) {
            int sum = 0;
            for (int ki = 0; ki < ksize; ++ki) {
                for (int kj = 0; kj < ksize; ++kj) {
                    int xi = i * stride + ki * dilation - pad;
                    int yj = j * stride + kj * dilation - pad;
                    int val = 0;
                    if (xi >= 0 && xi < rows && yj >= 0 && yj < cols) {
                        val = get(xi, yj);
                    }
                    sum += val * kernel.get(ki, kj);
                }
            }
            result.set(i, j, sum);
        }
    }
    return result;
}

// 打印矩阵
void Matrix::print() const {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << get(i, j) << " ";
        }
        std::cout << std::endl;
    }
}

// 输入矩阵
void Matrix::input() {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int val;
            std::cin >> val;
            set(i, j, val);
        }
    }
}
