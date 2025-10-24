#include <iostream>
#include <sstream>
#include <vector>
#include <cstdint>
#include "bit_io.h"
#include "huffman.h"

using namespace fc;

static bool test_bitio_cross_byte()
{
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    BitWriter bw(ss);
    bw.writeBits(0b101, 3);   // 3 bits
    bw.writeBits(0b10011, 5); // cross into next byte
    bw.writeBits(0x1A5, 9);   // cross multiple bytes
    bw.flush();

    BitReader br(ss);
    uint32_t v = 0;
    if (!br.readBits(3, v) || v != 0b101)
        return false;
    if (!br.readBits(5, v) || v != 0b10011)
        return false;
    if (!br.readBits(9, v) || v != 0x1A5)
        return false;
    return true;
}

static bool test_bitio_unaligned_tail()
{
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    BitWriter bw(ss);
    bw.writeBits(0x5A, 7); // 0b1011010
    bw.flush();

    BitReader br(ss);
    uint32_t v = 0;
    if (!br.readBits(7, v) || v != 0x5A)
        return false;
    // One padding bit (0) remains in the final byte; reading 1 bit should succeed and be 0
    if (!br.readBits(1, v) || v != 0)
        return false;
    // Now no more bits available
    uint32_t dummy;
    if (br.readBits(1, dummy))
        return false;
    return true;
}

static bool test_bitio_many_segments()
{
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    BitWriter bw(ss);
    const int n = 50;
    std::vector<std::pair<uint32_t, int>> segs;
    segs.reserve(n);
    // deterministic pseudo-values
    uint32_t seed = 1234567u;
    for (int i = 0; i < n; ++i)
    {
        seed = seed * 1103515245u + 12345u;
        int w = 1 + (seed % 17); // width 1..17
        seed = seed * 1103515245u + 12345u;
        uint32_t val = (seed >> 8) & 0xFFFFu;
        if (w < 32)
            val &= ((1u << w) - 1u);
        segs.push_back({val, w});
        bw.writeBits(val, w);
    }
    bw.flush();

    BitReader br(ss);
    for (int i = 0; i < n; ++i)
    {
        uint32_t v = 0;
        if (!br.readBits(segs[i].second, v))
            return false;
        if (v != segs[i].first)
            return false;
    }
    return true;
}

static bool test_huffman_roundtrip()
{
    // A small synthetic alphabet of size 6 with classic frequencies
    std::vector<uint32_t> freqs = {5, 7, 10, 15, 20, 45};
    HuffmanCodec hc;
    if (!hc.build(freqs))
        return false;

    std::vector<uint16_t> seq = {5, 4, 3, 2, 1, 0, 5, 5, 2, 3, 4, 1, 0};
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    BitWriter bw(ss);
    for (auto s : seq)
    {
        if (!hc.encode(s, bw))
            return false;
    }
    bw.flush();

    BitReader br(ss);
    for (auto expected : seq)
    {
        uint16_t got = 0;
        if (!hc.decode(br, got))
            return false;
        if (got != expected)
            return false;
    }
    return true;
}

int main()
{
    int passed = 0, total = 4;
    if (test_bitio_cross_byte())
        ++passed;
    else
        std::cout << "bitio_cross_byte FAILED\n";
    if (test_bitio_unaligned_tail())
        ++passed;
    else
        std::cout << "bitio_unaligned_tail FAILED\n";
    if (test_bitio_many_segments())
        ++passed;
    else
        std::cout << "bitio_many_segments FAILED\n";
    if (test_huffman_roundtrip())
        ++passed;
    else
        std::cout << "huffman_roundtrip FAILED\n";

    if (passed == total)
    {
        std::cout << "ALL TESTS PASSED\n";
        return 0;
    }
    else
    {
        std::cout << passed << "/" << total << " tests passed\n";
        return 1;
    }
}
