#include "bit_io.h"
#include <ostream>
#include <istream>

namespace fc
{

    BitWriter::BitWriter(std::ostream &out)
        : out_(&out), buf_(0), bitCount_(0), totalBits_(0) {}

    void BitWriter::writeBits(uint32_t value, int nbits)
    {
        if (nbits <= 0)
            return;
        // Mask to nbits
        if (nbits < 32)
            value &= ((1u << nbits) - 1u);
        buf_ |= (static_cast<uint64_t>(value) << bitCount_);
        bitCount_ += nbits;
        totalBits_ += static_cast<size_t>(nbits);
        while (bitCount_ >= 8)
        {
            unsigned char byte = static_cast<unsigned char>(buf_ & 0xFFu);
            out_->put(static_cast<char>(byte));
            buf_ >>= 8;
            bitCount_ -= 8;
        }
    }

    void BitWriter::alignToByte()
    {
        int rem = bitCount_ & 7;
        if (rem != 0)
        {
            int pad = 8 - rem;
            writeBits(0, pad);
        }
    }

    void BitWriter::flush()
    {
        if (bitCount_ > 0)
        {
            unsigned char byte = static_cast<unsigned char>(buf_ & 0xFFu);
            out_->put(static_cast<char>(byte));
            buf_ >>= 8;
            bitCount_ = 0;
        }
        out_->flush();
    }

    BitReader::BitReader(std::istream &in)
        : in_(&in), buf_(0), bitCount_(0), eof_(false) {}

    bool BitReader::readBits(int nbits, uint32_t &value)
    {
        if (nbits <= 0)
        {
            value = 0;
            return true;
        }
        while (bitCount_ < nbits && !eof_)
        {
            int ch = in_->get();
            if (ch == EOF)
            {
                eof_ = true;
                break;
            }
            buf_ |= (static_cast<uint64_t>(static_cast<unsigned char>(ch)) << bitCount_);
            bitCount_ += 8;
        }
        if (bitCount_ < nbits)
        {
            // Not enough bits
            return false;
        }
        uint32_t mask = (nbits >= 32) ? 0xFFFFFFFFu : ((1u << nbits) - 1u);
        value = static_cast<uint32_t>(buf_ & mask);
        buf_ >>= nbits;
        bitCount_ -= nbits;
        return true;
    }

    bool BitReader::alignToByte()
    {
        int rem = bitCount_ & 7;
        if (rem == 0)
            return !eof_ || bitCount_ > 0;
        uint32_t dummy;
        return readBits(8 - rem, dummy);
    }

} // namespace fc
