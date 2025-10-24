#include "deflate.h"
#include "bit_io.h"
#include "huffman.h"
#include <vector>
#include <string>
#include <ostream>
#include <cstdint>

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
            std::vector<std::pair<uint16_t, uint32_t>> llFreqs;   // (symbol, freq)
            std::vector<std::pair<uint16_t, uint32_t>> distFreqs; // (symbol, freq)
        };

        // Little-endian write helpers
        inline void writeU16LE(std::ostream &out, uint16_t v)
        {
            unsigned char buf[2];
            buf[0] = static_cast<unsigned char>(v & 0xFFu);
            buf[1] = static_cast<unsigned char>((v >> 8) & 0xFFu);
            out.write(reinterpret_cast<const char *>(buf), 2);
        }

        inline void writeU32LE(std::ostream &out, uint32_t v)
        {
            unsigned char buf[4];
            buf[0] = static_cast<unsigned char>(v & 0xFFu);
            buf[1] = static_cast<unsigned char>((v >> 8) & 0xFFu);
            buf[2] = static_cast<unsigned char>((v >> 16) & 0xFFu);
            buf[3] = static_cast<unsigned char>((v >> 24) & 0xFFu);
            out.write(reinterpret_cast<const char *>(buf), 4);
        }

        inline void writeU64LE(std::ostream &out, uint64_t v)
        {
            unsigned char buf[8];
            for (int i = 0; i < 8; ++i)
            {
                buf[i] = static_cast<unsigned char>((v >> (i * 8)) & 0xFFu);
            }
            out.write(reinterpret_cast<const char *>(buf), 8);
        }

        bool writeHeader(std::ostream &out, const FileHeader &hdr, std::string *err)
        {
            writeU32LE(out, MAGIC);
            writeU16LE(out, hdr.version);
            writeU32LE(out, hdr.windowSize);
            writeU64LE(out, hdr.originalSize);

            uint16_t llCount = static_cast<uint16_t>(hdr.llFreqs.size());
            writeU16LE(out, llCount);
            for (const auto &p : hdr.llFreqs)
            {
                writeU16LE(out, p.first);
                writeU32LE(out, p.second);
            }

            uint16_t distCount = static_cast<uint16_t>(hdr.distFreqs.size());
            writeU16LE(out, distCount);
            for (const auto &p : hdr.distFreqs)
            {
                writeU16LE(out, p.first);
                writeU32LE(out, p.second);
            }

            if (!out)
            {
                if (err)
                    *err = "writeHeader: I/O error during write";
                return false;
            }
            return true;
        }
    }

    bool deflateStream(std::istream &in, std::ostream & /*out*/, const DeflateOptions & /*opt*/, std::string *err)
    {
        // Skeleton only: not implemented yet
        if (err)
            *err = "deflateStream: not implemented (skeleton)";
        return false;
    }

} // namespace fc
