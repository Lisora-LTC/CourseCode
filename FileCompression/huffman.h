#pragma once
#include <cstdint>
#include <vector>
#include <iosfwd>
#include <cstddef>
#include <unordered_map>

namespace fc
{

    struct Code
    {
        uint32_t bits = 0; // LSB-first bits
        uint8_t bitlen = 0;
    };

    // In DEFLATE: literal/length alphabet size typically 286 (0-285), distance 30 (0-29)
    constexpr size_t LL_ALPHABET_SIZE = 286;
    constexpr size_t DIST_ALPHABET_SIZE = 30;

    class BitWriter; // fwd
    class BitReader; // fwd

    class HuffmanCodec
    {
    public:
        HuffmanCodec() = default;
        // Build canonical codes from freqs; returns false if all freqs are zero
        bool build(const std::vector<uint32_t> &freqs);
        // Encode a symbol using the built table
        bool encode(uint16_t symbol, BitWriter &bw) const;
        // Decode a symbol from bitstream
        bool decode(BitReader &br, uint16_t &symbol) const;
        size_t size() const;

    private:
        // Canonical code table: index by symbol
        std::vector<Code> codes_;
        struct DecNode
        {
            int left = -1;
            int right = -1;
            int symbol = -1;
            bool isLeaf = false;
        };
        std::vector<DecNode> decNodes_; // LSB-first decode trie, root at index 0
    };

    inline size_t HuffmanCodec::size() const { return codes_.size(); }

} // namespace fc
