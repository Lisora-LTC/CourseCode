#include "mat.h"
#include <stdexcept>
#include <limits> // 添加这一行
using namespace std;

// 独立的判断函数
bool isValidSize(int rows, int cols) {
    return rows > 0 && cols > 0;
}

// 新的构造函数，接收一维数组指针和长度
Matrix::Matrix(int rowsinn, int colsinn) : rows(rowsinn), cols(colsinn), data(0) {
    if (!isValidSize(rowsinn, colsinn)) {
        throw std::invalid_argument("矩阵的行数和列数必须大于0。");
    }
    data = new int[rows * cols];
}

Matrix::~Matrix() {
    delete[] data;
}

// 获取矩阵的行数和列数
int Matrix::getRows() const { return rows; }
int Matrix::getCols() const { return cols; }

// 输入矩阵
void Matrix::input() {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int val;
            while (true) {
                cin >> val;
                if (cin.fail()) {
                    cin.clear(); // 清除错误标志
                    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 丢弃错误输入
                    cout << "输入无效，请重新输入一个整数: ";
                } else {
                    break;
                }
            }
            set(i, j, val);
        }
    }
}

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

// 矩阵归一化方法
void Matrix::normalize(int factor) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int value = get(i, j) / factor;
            set(i, j, value);
        }
    }
}

 // 将矩阵转换为OpenCV灰度图像
 cv::Mat Matrix::toGrayImage() const {
     using namespace cv; // 在函数内部使用命名空间
     Mat resultImage = Mat::zeros(rows, cols, CV_8UC1);
     for (int i = 0; i < rows; i++) {
         for (int j = 0; j < cols; j++) {
             int value = get(i, j);
             // 确保灰度值在有效范围内
             value = std::min(255, std::max(0, value));
             resultImage.at<uchar>(i, j) = static_cast<uchar>(value);
         }
     }
     return resultImage;
 }

// Otsu算法实现
Matrix Matrix::otsuThreshold() const {
    // 1. 统计直方图
    int hist[256] = {0};
    int total = rows * cols;
    for (int i = 0; i < rows * cols; ++i) {
        int val = data[i];
        if (val < 0) val = 0;
        if (val > 255) val = 255;
        hist[val]++;
    }

    // 2. 计算总像素数
    double sum = 0;
    for (int t = 0; t < 256; ++t)
        sum += t * hist[t];

    double sumB = 0;
    int wB = 0;
    int wF = 0;

    double varMax = 0;
    int threshold = 0;

    for (int t = 0; t < 256; ++t) {
        wB += hist[t];
        if (wB == 0) continue;
        wF = total - wB;
        if (wF == 0) break;

        sumB += t * hist[t];

        double mB = sumB / wB;
        double mF = (sum - sumB) / wF;

        double varBetween = (double)wB * (double)wF * (mB - mF) * (mB - mF);

        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = t;
        }
    }

    // 3. 应用阈值，生成二值化矩阵
    Matrix result(rows, cols);
    for (int i = 0; i < rows * cols; ++i) {
        result.data[i] = (data[i] > threshold) ? 255 : 0;
    }

    return result;
}

// 打印矩阵
void Matrix::print() const {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << get(i, j) << " ";
        }
        cout << endl;
    }
}

