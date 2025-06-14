#include "Solitare.h"
#include "BoardsConfig.h"
#include <unordered_map>
#include <bitset>
#include <algorithm>

// 自定义哈希和相等比较，用于 Coord (std::pair<int,int>)
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

using State = uint64_t;                // 用于位掩码表示棋盘状态
const int MAX_MOVES = 100;             // 最大搜索深度
static std::vector<MoveRecord> moves;  // 存储所有可能的跳跃(move.from, move.mid, move.to)

// 生成所有合法跳跃三元组
static void initMoves(int n, const std::vector<Coord>& coords) {
    if (!moves.empty()) return;
    // 坐标到索引映射
    std::unordered_map<Coord,int, CoordHash, CoordEqual> idx;
    for (int i = 0; i < n; ++i) {
        idx[coords[i]] = i;
    }
    int B = 70; // 单格像素间距
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

// 计算启发函数: 当前棋子数减1
static int heuristic(State s) {
    return (int)std::bitset<64>(s).count() - 1;
}

// 判断是否目标状态(仅剩1颗棋子)
static bool isGoal(State s) {
    return std::bitset<64>(s).count() == 1;
}

// 将棋盘转换为位掩码
static State encode(const std::vector<int>& st) {
    State s = 0;
    for (int i = 0; i < (int)st.size(); ++i) {
        if (st[i]) s |= (State(1) << i);
    }
    return s;
}

// DFS 辅助
static bool dfs(State s, int g, int bound, int& nextBound,
                std::vector<MoveRecord>& path) {
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
        // 检查合法性: from和mid有子, to无子
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

// IDA*搜索，返回完整路径
static bool idaStar(State start, std::vector<MoveRecord>& result, int n, const std::vector<Coord>& coords) {
    initMoves(n, coords);
    int bound = heuristic(start);
    while (true) {
        int nextBound = std::numeric_limits<int>::max();
        std::vector<MoveRecord> path;
        if (dfs(start, 0, bound, nextBound, path)) {
            result = path;
            return true;
        }
        if (nextBound == std::numeric_limits<int>::max()) return false;
        bound = nextBound;
    }
}

// 对外接口: 返回第一步走法
MoveRecord searchBestMove(const Chessboard& board) {
    int n = board.getBlockCount();
    std::vector<int> st(n);
    for (int i = 0; i < n; ++i) st[i] = board.hasPieceAt(i) ? 1 : 0;
    State s = encode(st);
    std::vector<MoveRecord> path;
    // 使用更新后的 EnglishCoords，现在包含了所有33个位置
    if (idaStar(s, path, n, EnglishCoords) && !path.empty()) {
        return path[0];
    }
    return MoveRecord(-1, -1, -1);
}
