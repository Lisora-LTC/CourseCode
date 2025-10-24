#include "inflate.h"
#include "bit_io.h"
#include "huffman.h"
#include <string>
#include <istream>
#include <ostream>
#include <cstdint>
#include <vector>

namespace fc
{
    namespace
    {
        constexpr uint32_t MAGIC = 0x31304346u; // "FC01" in little-endian

        struct FileHeader
        {
            uint16_t version = 1;
            uint32_t windowSize = 32768;
            uint64_t originalSize = 0;
            std::vector<std::pair<uint16_t, uint32_t>> llFreqs;
            std::vector<std::pair<uint16_t, uint32_t>> distFreqs;
        };

        // Little-endian read helpers
        inline bool readU16LE(std::istream &in, uint16_t &v)
        {
            unsigned char buf[2];
            in.read(reinterpret_cast<char *>(buf), 2);
            if (in.gcount() != 2)
                return false;
            v = static_cast<uint16_t>(buf[0]) | (static_cast<uint16_t>(buf[1]) << 8);
            return true;
        }

        inline bool readU32LE(std::istream &in, uint32_t &v)
        {
            unsigned char buf[4];
            in.read(reinterpret_cast<char *>(buf), 4);
            if (in.gcount() != 4)
                return false;
            v = static_cast<uint32_t>(buf[0]) | (static_cast<uint32_t>(buf[1]) << 8) |
                (static_cast<uint32_t>(buf[2]) << 16) | (static_cast<uint32_t>(buf[3]) << 24);
            return true;
        }

        inline bool readU64LE(std::istream &in, uint64_t &v)
        {
            unsigned char buf[8];
            in.read(reinterpret_cast<char *>(buf), 8);
            if (in.gcount() != 8)
                return false;
            v = 0;
            for (int i = 0; i < 8; ++i)
            {
                v |= static_cast<uint64_t>(buf[i]) << (i * 8);
            }
            return true;
        }

        bool readHeader(std::istream &in, FileHeader &hdr, std::string *err)
        {
            uint32_t magic = 0;
            if (!readU32LE(in, magic) || magic != MAGIC)
            {
                if (err)
                    *err = "readHeader: invalid magic or truncated";
                return false;
            }

            if (!readU16LE(in, hdr.version))
            {
                if (err)
                    *err = "readHeader: failed to read version";
                return false;
            }

            if (!readU32LE(in, hdr.windowSize))
            {
                if (err)
                    *err = "readHeader: failed to read windowSize";
                return false;
            }

            if (!readU64LE(in, hdr.originalSize))
            {
                if (err)
                    *err = "readHeader: failed to read originalSize";
                return false;
            }

            uint16_t llCount = 0;
            if (!readU16LE(in, llCount))
            {
                if (err)
                    *err = "readHeader: failed to read llFreqCount";
                return false;
            }
            hdr.llFreqs.clear();
            hdr.llFreqs.reserve(llCount);
            for (uint16_t i = 0; i < llCount; ++i)
            {
                uint16_t sym = 0;
                uint32_t freq = 0;
                if (!readU16LE(in, sym) || !readU32LE(in, freq))
                {
                    if (err)
                        *err = "readHeader: failed to read LL freq entry";
                    return false;
                }
                hdr.llFreqs.push_back({sym, freq});
            }

            uint16_t distCount = 0;
            if (!readU16LE(in, distCount))
            {
                if (err)
                    *err = "readHeader: failed to read distFreqCount";
                return false;
            }
            hdr.distFreqs.clear();
            hdr.distFreqs.reserve(distCount);
            for (uint16_t i = 0; i < distCount; ++i)
            {
                uint16_t sym = 0;
                uint32_t freq = 0;
                if (!readU16LE(in, sym) || !readU32LE(in, freq))
                {
                    if (err)
                        *err = "readHeader: failed to read dist freq entry";
                    return false;
                }
                hdr.distFreqs.push_back({sym, freq});
            }

            return true;
        }
    }

    bool inflateStream(std::istream &in, std::ostream &out, std::string *err)
    {
        // Read header
        FileHeader hdr;
        if (!readHeader(in, hdr, err))
        {
            return false;
        }

        // Rebuild frequency tables from header
        std::vector<uint32_t> llFreqs(286, 0);
        std::vector<uint32_t> distFreqs(30, 0);

        for (const auto &p : hdr.llFreqs)
        {
            if (p.first < llFreqs.size())
            {
                llFreqs[p.first] = p.second;
            }
        }
        for (const auto &p : hdr.distFreqs)
        {
            if (p.first < distFreqs.size())
            {
                distFreqs[p.first] = p.second;
            }
        }

        // Build Huffman codecs
        HuffmanCodec llCodec, distCodec;
        if (!llCodec.build(llFreqs))
        {
            if (err)
                *err = "inflateStream: failed to build LL Huffman tree";
            return false;
        }

        // Check if we have any distance codes
        bool hasMatches = false;
        for (auto f : distFreqs)
        {
            if (f > 0)
            {
                hasMatches = true;
                break;
            }
        }
        if (hasMatches)
        {
            if (!distCodec.build(distFreqs))
            {
                if (err)
                    *err = "inflateStream: failed to build Distance Huffman tree";
                return false;
            }
        }

        // Decode bit stream
        BitReader br(in);
        std::vector<uint8_t> output;
        output.reserve(static_cast<size_t>(hdr.originalSize));

        while (true)
        {
            uint16_t sym = 0;
            if (!llCodec.decode(br, sym))
            {
                if (err)
                    *err = "inflateStream: failed to decode LL symbol";
                return false;
            }

            if (sym < 256)
            {
                // Literal
                output.push_back(static_cast<uint8_t>(sym));
            }
            else if (sym == 256)
            {
                // EOB
                break;
            }
            else if (sym >= 257)
            {
                // Match: decode length and distance
                // Read length (9 bits)
                uint32_t lenVal = 0;
                if (!br.readBits(9, lenVal))
                {
                    if (err)
                        *err = "inflateStream: failed to read length";
                    return false;
                }
                uint16_t length = static_cast<uint16_t>(lenVal);

                // Decode distance symbol (if matches exist)
                if (hasMatches)
                {
                    uint16_t distSym = 0;
                    if (!distCodec.decode(br, distSym))
                    {
                        if (err)
                            *err = "inflateStream: failed to decode distance symbol";
                        return false;
                    }
                }

                // Read distance (15 bits)
                uint32_t distVal = 0;
                if (!br.readBits(15, distVal))
                {
                    if (err)
                        *err = "inflateStream: failed to read distance";
                    return false;
                }
                uint16_t distance = static_cast<uint16_t>(distVal);

                // Perform LZ77 backreference copy (supports overlapping)
                if (distance > output.size())
                {
                    if (err)
                        *err = "inflateStream: distance exceeds output size";
                    return false;
                }

                size_t start = output.size() - distance;
                for (uint16_t i = 0; i < length; ++i)
                {
                    output.push_back(output[start + i]);
                }
            }
            else
            {
                if (err)
                    *err = "inflateStream: invalid symbol";
                return false;
            }
        }

        // Verify output size
        if (output.size() != hdr.originalSize)
        {
            if (err)
            {
                *err = "inflateStream: output size mismatch (expected " +
                       std::to_string(hdr.originalSize) + ", got " +
                       std::to_string(output.size()) + ")";
            }
            return false;
        }

        // Write output
        out.write(reinterpret_cast<const char *>(output.data()), output.size());
        if (!out)
        {
            if (err)
                *err = "inflateStream: failed to write output";
            return false;
        }

        return true;
    }

} // namespace fc
