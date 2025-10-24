#include "lz77.h"
#include <istream>
#include <vector>
#include <algorithm>
#include <cstring>

namespace fc
{
    namespace
    {
        // 3-byte rolling hash constant
        constexpr uint32_t HASH_SHIFT = 5;
        constexpr uint32_t HASH_MASK = 0xFFFFu; // 16-bit hash for reasonable table size

        inline uint32_t hashFunc(uint8_t a, uint8_t b, uint8_t c)
        {
            uint32_t h = static_cast<uint32_t>(a);
            h = ((h << HASH_SHIFT) ^ static_cast<uint32_t>(b)) & HASH_MASK;
            h = ((h << HASH_SHIFT) ^ static_cast<uint32_t>(c)) & HASH_MASK;
            return h;
        }

        struct Match
        {
            uint16_t length = 0;
            uint16_t distance = 0;
        };

        // Find longest match in sliding window using hash chain
        Match findLongestMatch(const std::vector<uint8_t> &buf,
                               size_t pos,
                               const std::vector<std::vector<uint32_t>> &hashTable,
                               uint32_t windowSize,
                               uint32_t maxCandidates,
                               uint16_t minMatch,
                               uint16_t maxMatch)
        {
            Match best{0, 0};
            if (pos + minMatch > buf.size())
                return best;

            // Compute hash of current 3-byte sequence
            uint32_t h = hashFunc(buf[pos], buf[pos + 1], buf[pos + 2]);
            const auto &chain = hashTable[h];

            // Search window start
            size_t windowStart = (pos > windowSize) ? (pos - windowSize) : 0;

            // Limit candidates
            uint32_t checked = 0;
            for (auto it = chain.rbegin(); it != chain.rend() && checked < maxCandidates; ++it)
            {
                uint32_t candPos = *it;
                if (candPos >= pos)
                    continue; // future or self
                if (candPos < windowStart)
                    break; // out of window (chain is sorted)

                ++checked;

                // Compute distance
                uint16_t dist = static_cast<uint16_t>(pos - candPos);

                // Find match length
                size_t maxLen = std::min<size_t>(maxMatch, buf.size() - pos);
                size_t len = 0;
                while (len < maxLen && buf[candPos + len] == buf[pos + len])
                {
                    ++len;
                }

                if (len >= minMatch)
                {
                    // Prefer longer match; if tie, prefer smaller distance
                    if (len > best.length || (len == best.length && dist < best.distance))
                    {
                        best.length = static_cast<uint16_t>(len);
                        best.distance = dist;
                    }
                }
            }

            return best;
        }
    }

    bool LZ77Encoder::encode(std::istream &in, std::vector<Token> &outTokens, size_t *inputSize)
    {
        outTokens.clear();

        // Read entire input into buffer for efficient lookback
        std::vector<uint8_t> buf;
        buf.reserve(1024 * 64); // start with 64KB
        while (true)
        {
            int ch = in.get();
            if (ch == EOF)
                break;
            buf.push_back(static_cast<uint8_t>(ch));
        }

        if (inputSize)
            *inputSize = buf.size();
        if (buf.empty())
            return true;

        // Build hash table: hash -> list of positions
        // Use 65K buckets (16-bit hash)
        std::vector<std::vector<uint32_t>> hashTable(65536);

        size_t pos = 0;
        while (pos < buf.size())
        {
            // Try to find match if we have at least minMatch bytes ahead
            if (pos + opt_.minMatch <= buf.size())
            {
                Match m = findLongestMatch(buf, pos, hashTable, opt_.windowSize, opt_.maxCandidates, opt_.minMatch, opt_.maxMatch);
                if (m.length >= opt_.minMatch)
                {
                    // Emit match token
                    outTokens.push_back(Token::makeMatch(m.length, m.distance));

                    // Insert all positions in matched region into hash table (for future lookups)
                    for (uint16_t i = 0; i < m.length && pos + i + 2 < buf.size(); ++i)
                    {
                        uint32_t h = hashFunc(buf[pos + i], buf[pos + i + 1], buf[pos + i + 2]);
                        hashTable[h].push_back(static_cast<uint32_t>(pos + i));
                    }

                    pos += m.length;
                    continue;
                }
            }

            // No match or not enough bytes: emit literal
            outTokens.push_back(Token::makeLiteral(buf[pos]));

            // Insert current position into hash table if possible
            if (pos + 2 < buf.size())
            {
                uint32_t h = hashFunc(buf[pos], buf[pos + 1], buf[pos + 2]);
                hashTable[h].push_back(static_cast<uint32_t>(pos));
            }

            ++pos;
        }

        return true;
    }

} // namespace fc
