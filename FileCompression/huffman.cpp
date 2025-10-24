#include "huffman.h"
#include "bit_io.h"
#include <queue>
#include <algorithm>
#include <cstdint>

namespace fc
{
    namespace
    {
        struct BuildNode
        {
            uint64_t freq = 0;
            int left = -1;
            int right = -1;
            int symbol = -1; // >=0 for leaf
        };

        struct HeapItem
        {
            uint64_t freq;
            int index;
            int tie; // deterministic ordering
            bool operator<(const HeapItem &o) const
            {
                if (freq != o.freq)
                    return freq > o.freq; // min-heap by freq
                return tie > o.tie;       // smaller tie first
            }
        };

        static inline uint32_t reverseBits(uint32_t v, int bits)
        {
            uint32_t r = 0;
            for (int i = 0; i < bits; ++i)
            {
                r |= ((v >> i) & 1u) << (bits - 1 - i);
            }
            return r;
        }
    }

    bool HuffmanCodec::build(const std::vector<uint32_t> &freqs)
    {
        if (freqs.empty())
            return false;
        int nonZeroCount = 0;
        for (auto f : freqs)
            if (f)
                ++nonZeroCount;
        if (nonZeroCount == 0)
            return false;

        std::vector<uint16_t> codeLen(freqs.size(), 0);
        if (nonZeroCount == 1)
        {
            for (size_t s = 0; s < freqs.size(); ++s)
            {
                if (freqs[s] != 0)
                {
                    codeLen[s] = 1; // single-symbol code length = 1
                    break;
                }
            }
        }
        else
        {
            // Build Huffman tree
            std::vector<BuildNode> nodes;
            nodes.reserve(freqs.size() * 2);
            std::priority_queue<HeapItem> pq;
            int tieCounter = 0;
            for (size_t s = 0; s < freqs.size(); ++s)
            {
                if (freqs[s] == 0)
                    continue;
                BuildNode leaf;
                leaf.freq = freqs[s];
                leaf.symbol = static_cast<int>(s);
                nodes.push_back(leaf);
                int idx = static_cast<int>(nodes.size() - 1);
                pq.push(HeapItem{leaf.freq, idx, static_cast<int>(s)});
            }
            while (pq.size() > 1)
            {
                HeapItem a = pq.top();
                pq.pop();
                HeapItem b = pq.top();
                pq.pop();
                BuildNode parent;
                parent.freq = a.freq + b.freq;
                parent.left = a.index;
                parent.right = b.index;
                nodes.push_back(parent);
                int pidx = static_cast<int>(nodes.size() - 1);
                pq.push(HeapItem{parent.freq, pidx, ++tieCounter});
            }
            int root = pq.top().index;

            struct Item
            {
                int idx;
                int depth;
            };
            std::vector<Item> st;
            st.push_back({root, 0});
            while (!st.empty())
            {
                Item cur = st.back();
                st.pop_back();
                const BuildNode &n = nodes[cur.idx];
                if (n.symbol >= 0)
                {
                    codeLen[static_cast<size_t>(n.symbol)] = static_cast<uint16_t>(cur.depth == 0 ? 1 : cur.depth);
                }
                else
                {
                    if (n.left >= 0)
                        st.push_back({n.left, cur.depth + 1});
                    if (n.right >= 0)
                        st.push_back({n.right, cur.depth + 1});
                }
            }
        }

        // Canonical assignment
        struct SymLen
        {
            uint16_t sym;
            uint16_t len;
        };
        std::vector<SymLen> v;
        v.reserve(freqs.size());
        for (uint16_t s = 0; s < freqs.size(); ++s)
            if (codeLen[s] > 0)
                v.push_back({s, codeLen[s]});
        std::sort(v.begin(), v.end(), [](const SymLen &a, const SymLen &b)
                  { return (a.len != b.len) ? (a.len < b.len) : (a.sym < b.sym); });

        codes_.assign(freqs.size(), Code{});
        uint32_t code = 0;
        uint16_t prevLen = 0;
        for (const auto &sl : v)
        {
            if (sl.len > prevLen)
            {
                code <<= (sl.len - prevLen);
                prevLen = sl.len;
            }
            Code c{};
            c.bitlen = static_cast<uint8_t>(sl.len);
            // store LSB-first by reversing canonical (MSB-first) bits
            c.bits = reverseBits(code, sl.len);
            codes_[sl.sym] = c;
            ++code;
        }

        // Build LSB-first decode trie
        decNodes_.clear();
        decNodes_.reserve(v.size() * 2 + 1);
        decNodes_.push_back(DecNode{}); // root 0
        for (const auto &sl : v)
        {
            const Code &c = codes_[sl.sym];
            int node = 0;
            for (int i = 0; i < c.bitlen; ++i)
            {
                uint32_t bit = (c.bits >> i) & 1u; // LSB-first traverse
                int &next = (bit == 0) ? decNodes_[node].left : decNodes_[node].right;
                if (next == -1)
                {
                    next = static_cast<int>(decNodes_.size());
                    decNodes_.push_back(DecNode{});
                }
                node = next;
            }
            decNodes_[node].isLeaf = true;
            decNodes_[node].symbol = sl.sym;
        }

        return true;
    }

    bool HuffmanCodec::encode(uint16_t symbol, BitWriter &bw) const
    {
        if (symbol >= codes_.size())
            return false;
        const Code &c = codes_[symbol];
        if (c.bitlen == 0)
            return false;
        bw.writeBits(c.bits, c.bitlen);
        return true;
    }

    bool HuffmanCodec::decode(BitReader &br, uint16_t &symbol) const
    {
        if (decNodes_.empty())
            return false;
        int node = 0;
        while (true)
        {
            if (decNodes_[node].isLeaf)
            {
                symbol = static_cast<uint16_t>(decNodes_[node].symbol);
                return true;
            }
            uint32_t bit = 0;
            if (!br.readBits(1, bit))
                return false;
            int next = (bit == 0) ? decNodes_[node].left : decNodes_[node].right;
            if (next == -1)
                return false;
            node = next;
        }
    }

} // namespace fc
