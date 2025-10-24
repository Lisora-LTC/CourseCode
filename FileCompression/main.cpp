// Minimal CLI wiring for DEFLATE-like compressor skeleton
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>

#include "deflate.h"
#include "inflate.h"

// 获取文件大小
static size_t getFileSize(std::ifstream &file)
{
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    return size;
}

static void print_usage(const char *exe)
{
    std::cout << "=========================================\n"
              << "  DEFLATE 压缩/解压缩工具\n"
              << "=========================================\n"
              << "使用方法:\n"
              << "  压缩:   " << exe << " <源文件> <目标文件> zip\n"
              << "  解压缩: " << exe << " <源文件> <目标文件> unzip\n"
              << "\n示例:\n"
              << "  " << exe << " data.txt data.fc zip\n"
              << "  " << exe << " data.fc restored.txt unzip\n"
              << "=========================================\n";
}

int main(int argc, char *argv[])
{
    std::string inPath, outPath, mode;

    // 检查是否通过命令行参数运行
    if (argc == 4)
    {
        // 命令行模式
        inPath = argv[1];
        outPath = argv[2];
        mode = argv[3];
    }
    else
    {
        // 交互模式
        std::cout << "=========================================\n"
                  << "  DEFLATE 压缩/解压缩工具\n"
                  << "=========================================\n\n";

        std::cout << "请输入源文件路径: ";
        std::getline(std::cin, inPath);

        std::cout << "请输入目标文件路径: ";
        std::getline(std::cin, outPath);

        std::cout << "请输入操作 (zip=压缩 / unzip=解压缩): ";
        std::getline(std::cin, mode);

        std::cout << "\n=========================================\n";
    }

    // 打开输入文件
    std::ifstream in(inPath, std::ios::binary);
    if (!in)
    {
        std::cerr << "❌ 错误: 无法打开输入文件 \"" << inPath << "\"\n";
        return 2;
    }

    // 获取输入文件大小
    size_t inputSize = getFileSize(in);

    std::ofstream out(outPath, std::ios::binary);
    if (!out)
    {
        std::cerr << "❌ 错误: 无法创建输出文件 \"" << outPath << "\"\n";
        return 3;
    }

    std::string err;
    auto startTime = std::chrono::high_resolution_clock::now();

    if (mode == "zip")
    {
        std::cout << "\n📦 开始压缩...\n";
        std::cout << "   源文件: " << inPath << " (" << inputSize << " 字节)\n";
        std::cout << "   目标文件: " << outPath << "\n";
        std::cout << "   正在处理中";
        std::cout.flush();

        fc::DeflateOptions opt{}; // defaults
        if (!fc::deflateStream(in, out, opt, &err))
        {
            std::cerr << "\n❌ 压缩失败: " << err << "\n";
            return 4;
        }

        out.close();
        std::ifstream outCheck(outPath, std::ios::binary);
        size_t outputSize = getFileSize(outCheck);
        outCheck.close();

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        double ratio = (inputSize > 0) ? (100.0 * outputSize / inputSize) : 0.0;

        std::cout << "\n✅ 压缩完成!\n";
        std::cout << "   原始大小: " << inputSize << " 字节\n";
        std::cout << "   压缩后大小: " << outputSize << " 字节\n";
        std::cout << "   压缩比: " << std::fixed << std::setprecision(2) << ratio << "%\n";
        std::cout << "   节省空间: " << (inputSize - outputSize) << " 字节\n";
        std::cout << "   用时: " << duration.count() << " 毫秒\n";

        return 0;
    }
    else if (mode == "unzip")
    {
        std::cout << "\n📂 开始解压缩...\n";
        std::cout << "   源文件: " << inPath << " (" << inputSize << " 字节)\n";
        std::cout << "   目标文件: " << outPath << "\n";
        std::cout << "   正在处理中";
        std::cout.flush();

        if (!fc::inflateStream(in, out, &err))
        {
            std::cerr << "\n❌ 解压缩失败: " << err << "\n";
            return 5;
        }

        out.close();
        std::ifstream outCheck(outPath, std::ios::binary);
        size_t outputSize = getFileSize(outCheck);
        outCheck.close();

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << "\n✅ 解压缩完成!\n";
        std::cout << "   压缩文件: " << inputSize << " 字节\n";
        std::cout << "   还原大小: " << outputSize << " 字节\n";
        std::cout << "   用时: " << duration.count() << " 毫秒\n";

        return 0;
    }
    else
    {
        std::cerr << "❌ 错误: 未知的操作指令 \"" << mode << "\"\n";
        std::cerr << "   请使用 \"zip\" 进行压缩，或使用 \"unzip\" 进行解压缩\n\n";
        print_usage(argv[0]);
        return 1;
    }
}
