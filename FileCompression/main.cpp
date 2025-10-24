// Minimal CLI wiring for DEFLATE-like compressor skeleton
#include <iostream>
#include <fstream>
#include <string>

#include "deflate.h"
#include "inflate.h"

static void print_usage(const char *exe)
{
    std::cout << "Usage:\n"
              << "  " << exe << " <src> <dst> zip    # compress src to dst\n"
              << "  " << exe << " <src> <dst> unzip  # decompress src to dst\n";
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        print_usage(argv[0]);
        return 1;
    }

    std::string inPath = argv[1];
    std::string outPath = argv[2];
    std::string mode = argv[3]; // must be "zip" or "unzip"
    std::ifstream in(inPath, std::ios::binary);
    if (!in)
    {
        std::cerr << "Failed to open input: " << inPath << "\n";
        return 2;
    }
    std::ofstream out(outPath, std::ios::binary);
    if (!out)
    {
        std::cerr << "Failed to open output: " << outPath << "\n";
        return 3;
    }

    std::string err;
    if (mode == "zip")
    {
        fc::DeflateOptions opt{}; // defaults
        if (!fc::deflateStream(in, out, opt, &err))
        {
            std::cerr << "Compress failed: " << err << "\n";
            return 4;
        }
        return 0;
    }
    else if (mode == "unzip")
    {
        if (!fc::inflateStream(in, out, &err))
        {
            std::cerr << "Decompress failed: " << err << "\n";
            return 5;
        }
        return 0;
    }
    else
    {
        print_usage(argv[0]);
        return 1;
    }
}
