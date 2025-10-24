#pragma once
#include <cstdint>
#include <vector>
#include <iosfwd>
#include <cstddef>

namespace fc
{

    struct LZ77Options
    {
        uint32_t windowSize = 32 * 1024; // 32KB
        uint16_t minMatch = 3;
        uint16_t maxMatch = 258;
        uint32_t maxCandidates = 256; // for future optimization
    };

    enum class TokenKind : uint8_t
    {
        Literal = 0,
        Match = 1
    };

    struct Token
    {
        TokenKind kind{};
        uint8_t literal = 0;   // valid when kind==Literal
        uint16_t length = 0;   // valid when kind==Match
        uint16_t distance = 0; // valid when kind==Match

        static Token makeLiteral(uint8_t v)
        {
            Token t;
            t.kind = TokenKind::Literal;
            t.literal = v;
            return t;
        }
        static Token makeMatch(uint16_t len, uint16_t dist)
        {
            Token t;
            t.kind = TokenKind::Match;
            t.length = len;
            t.distance = dist;
            return t;
        }
    };

    class LZ77Encoder
    {
    public:
        explicit LZ77Encoder(const LZ77Options &opt = {}) : opt_(opt) {}
        // Naive baseline: emits literals only for now; will be replaced with real matching.
        bool encode(std::istream &in, std::vector<Token> &outTokens, size_t *inputSize = nullptr);

    private:
        LZ77Options opt_{};
    };

} // namespace fc
