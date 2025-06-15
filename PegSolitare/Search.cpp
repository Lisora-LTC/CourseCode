#include "Solitare.h"
#include "BoardsConfig.h"
#include <unordered_map>
#include <bitset>
#include <algorithm>
#include <chrono>
#include <limits>

// 坐标类型的哈希和比较函数
struct CoordHash {
    size_t operator()(const Coord& c) const noexcept {
        return std::hash<int>()(c.first) ^ (std::hash<int>()(c.second) << 1);
    }
};
struct CoordEqual {
    bool operator()(const Coord& a, const Coord& b) const noexcept {
        return a.first == b.first && a.second == b.second;
    }
};

// 搜索相关类型定义和常量
using State = uint64_t;
const int MAX_MOVES = 100;
const int SEARCH_TIMEOUT_MS = 1000;
static std::vector<MoveRecord> moves;
static std::chrono::steady_clock::time_point searchStartTime;
static bool searchTimedOut = false;

// 生成所有合法跳跃组合
static void initMoves(int n, const std::vector<Coord>& coords) {
    if (!moves.empty()) return;
    std::unordered_map<Coord,int, CoordHash, CoordEqual> idx;
    for (int i = 0; i < n; ++i) {
        idx[coords[i]] = i;
    }
    int B = 70;
    int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (int i = 0; i < n; ++i) {
        int x = coords[i].first;
        int y = coords[i].second;
        for (int di = 0; di < 4; ++di) {
            int dx = dirs[di][0];
            int dy = dirs[di][1];
            Coord mid{x + dx*B, y + dy*B};
            Coord to {x + dx*2*B, y + dy*2*B};
            auto it1 = idx.find(mid);
            auto it2 = idx.find(to);
            if (it1 != idx.end() && it2 != idx.end()) {
                moves.emplace_back(i, it1->second, it2->second);
            }
        }
    }
}

// 启发函数计算
static int heuristic(State s) {
    return (int)std::bitset<64>(s).count() - 1;
}

// 目标状态判断
static bool isGoal(State s) {
    return std::bitset<64>(s).count() == 1;
}

// 棋盘状态编码
static State encode(const std::vector<int>& st) {
    State s = 0;
    for (int i = 0; i < (int)st.size(); ++i) {
        if (st[i]) s |= (State(1) << i);
    }
    return s;
}

// 深度优先搜索
static bool dfs(State s, int g, int bound, int& nextBound,
                std::vector<MoveRecord>& path) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - searchStartTime);
    if (elapsed.count() >= SEARCH_TIMEOUT_MS) {
        searchTimedOut = true;
        return false;
    }
    
    int f = g + heuristic(s);
    if (f > bound) {
        nextBound = std::min(nextBound, f);
        return false;
    }
    if (isGoal(s)) return true;
    for (size_t mi = 0; mi < moves.size(); ++mi) {
        const MoveRecord& m = moves[mi];
        int from = m.fromIndex;
        int mid = m.middleIndex;
        int to   = m.toIndex;
        State bit = State(1);
        if ((s & (bit<<from)) && (s & (bit<<mid)) && !(s & (bit<<to))) {
            State ns = s;
            ns ^= (bit<<from) | (bit<<mid) | (bit<<to);
            path.emplace_back(from, mid, to);
            if (dfs(ns, g+1, bound, nextBound, path)) return true;
            path.pop_back();
        }
    }
    return false;
}

// IDA*搜索算法
static bool idaStar(State start, std::vector<MoveRecord>& result, int n, const std::vector<Coord>& coords) {
    initMoves(n, coords);
    searchTimedOut = false;
    int bound = heuristic(start);
    while (true) {
        if (searchTimedOut) {
            return false;
        }
        int nextBound = std::numeric_limits<int>::max();
        std::vector<MoveRecord> path;
        if (dfs(start, 0, bound, nextBound, path)) {
            result = path;
            return true;
        }
        if (searchTimedOut) {
            return false;
        }
        if (nextBound == std::numeric_limits<int>::max()) return false;
        bound = nextBound;
    }
}

// 搜索最佳移动
MoveRecord searchBestMove(const Chessboard& board) {
    searchStartTime = std::chrono::steady_clock::now();
    
    int n = board.getBlockCount();
    std::vector<int> st(n);
    for (int i = 0; i < n; ++i) st[i] = board.hasPieceAt(i) ? 1 : 0;
    State s = encode(st);
    std::vector<MoveRecord> path;
    if (idaStar(s, path, n, EnglishCoords) && !path.empty()) {
        return path[0];
    }
    return MoveRecord(-1, -1, -1);
}
