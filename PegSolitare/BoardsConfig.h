#ifndef BOARDS_CONFIG_H
#define BOARDS_CONFIG_H

#include <vector>
#include <utility>
#include <unordered_map>
#include <string>

using Coord = std::pair<int,int>;

// 英式棋盘坐标列表
static const std::vector<Coord> EnglishCoords = {
    // 第1行
    {535, 165}, {605, 165}, {675, 165},
    // 第2行
    {535, 235}, {605, 235}, {675, 235},
    // 第3行
    {395, 305}, {465, 305}, {535, 305}, {605, 305}, {675, 305}, {745, 305}, {815, 305},
    // 第4行
    {395, 375}, {465, 375}, {535, 375}, {605, 375}, {675, 375}, {745, 375}, {815, 375},
    // 第5行
    {395, 445}, {465, 445}, {535, 445}, {605, 445}, {675, 445}, {745, 445}, {815, 445},
    // 第6行
    {535, 515}, {605, 515}, {675, 515},
    // 第7行
    {535, 585}, {605, 585}, {675, 585}
};

// 可以在这里添加更多棋盘类型的坐标
// static const std::vector<Coord> FrenchCoords = { ... };

static const std::unordered_map<std::string, std::vector<Coord>> AllBoards = {
    {"English", EnglishCoords}
    // {"French",  FrenchCoords},
};

#endif // BOARDS_CONFIG_H
