#ifndef BOARDS_CONFIG_H
#define BOARDS_CONFIG_H

#include <vector>
#include <utility>
#include <unordered_map>
#include <string>

using Coord = std::pair<int,int>;

// 英式棋盘(十字形)坐标列表 - 720p适配
// 标准English board: 7x7网格的十字形，总共33个位置
// 格子大小70px，棋盘总宽度: 7*70=490px, 居中: (1280-490)/2=395
// 棋盘总高度: 7*70=490px, 顶部蓝条100px后居中: 100+(720-100-490)/2=165
static const std::vector<Coord> EnglishCoords = {
    // 第1行 (row 0): 位置 2,3,4
    {535, 165}, {605, 165}, {675, 165},
    // 第2行 (row 1): 位置 2,3,4  
    {535, 235}, {605, 235}, {675, 235},
    // 第3行 (row 2): 位置 0,1,2,3,4,5,6
    {395, 305}, {465, 305}, {535, 305}, {605, 305}, {675, 305}, {745, 305}, {815, 305},
    // 第4行 (row 3): 位置 0,1,2,3,4,5,6 (包含中间位置3)
    {395, 375}, {465, 375}, {535, 375}, {605, 375}, {675, 375}, {745, 375}, {815, 375},
    // 第5行 (row 4): 位置 0,1,2,3,4,5,6
    {395, 445}, {465, 445}, {535, 445}, {605, 445}, {675, 445}, {745, 445}, {815, 445},
    // 第6行 (row 5): 位置 2,3,4
    {535, 515}, {605, 515}, {675, 515},
    // 第7行 (row 6): 位置 2,3,4
    {535, 585}, {605, 585}, {675, 585}
};

// 可以在这里添加更多棋盘类型的坐标
// static const std::vector<Coord> FrenchCoords = { ... };

static const std::unordered_map<std::string, std::vector<Coord>> AllBoards = {
    {"English", EnglishCoords}
    // {"French",  FrenchCoords},
};

#endif // BOARDS_CONFIG_H
