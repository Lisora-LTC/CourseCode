#pragma once
#include <cstdint>
#include <cstddef>
#include <iosfwd>

namespace fc
{

    class BitWriter
    {
    public:
        explicit BitWriter(std::ostream &out);
        // LSB-first: write lowest nbits of value into the stream
        void writeBits(uint32_t value, int nbits);
        // Pad to next byte with zeros and flush remaining bytes
        void alignToByte();
        void flush();
        size_t totalBits() const { return totalBits_; }

    private:
        std::ostream *out_;
        uint64_t buf_;
        int bitCount_;
        size_t totalBits_;
    };

    class BitReader
    {
    public:
        explicit BitReader(std::istream &in);
        // LSB-first: read nbits into value; return false on EOF/underflow
        bool readBits(int nbits, uint32_t &value);
        // Discard to next byte boundary; return false if already at EOF and no buffered bits
        bool alignToByte();
        bool eof() const { return eof_ && bitCount_ == 0; }

    private:
        std::istream *in_;
        uint64_t buf_;
        int bitCount_;
        bool eof_;
    };

} // namespace fc
