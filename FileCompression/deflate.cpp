#include "deflate.h"
#include "bit_io.h"
#include "huffman.h"
#include "lz77.h"
#include <vector>
#include <string>
#include <ostream>
#include <istream>
#include <sstream>
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

    bool deflateStream(std::istream &in, std::ostream &out, const DeflateOptions &opt, std::string *err)
    {
        // Pass 1: Run LZ77 to collect tokens and compute frequencies
        LZ77Encoder lz77(opt.lz);
        std::vector<Token> tokens;
        size_t inputSize = 0;

        if (!lz77.encode(in, tokens, &inputSize))
        {
            if (err)
                *err = "deflateStream: LZ77 encoding failed";
            return false;
        }

        // Build frequency tables for Literal/Length and Distance
        // LL alphabet: 0-255 (literals) + 256 (EOB) + 257-285 (lengths 3-258)
        // Distance alphabet: 0-29 (distances 1-32768, using base+extra bits scheme)
        std::vector<uint32_t> llFreqs(286, 0);
        std::vector<uint32_t> distFreqs(30, 0);

        for (const auto &t : tokens)
        {
            if (t.kind == TokenKind::Literal)
            {
                llFreqs[t.literal]++;
            }
            else if (t.kind == TokenKind::Match)
            {
                // Simplified: use fixed symbol for all matches
                // Symbol 257 represents "match follows"
                llFreqs[257]++;
                // Distance symbol: always use symbol 0 (we'll encode actual distance as extra bits)
                distFreqs[0]++;
            }
        }

        // EOB marker
        llFreqs[256]++;

        // Build Huffman codecs
        HuffmanCodec llCodec, distCodec;
        if (!llCodec.build(llFreqs))
        {
            if (err)
                *err = "deflateStream: failed to build LL Huffman tree";
            return false;
        }

        // Handle case where there are no matches (no distance codes needed)
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
                    *err = "deflateStream: failed to build Distance Huffman tree";
                return false;
            }
        }

        // Prepare header
        FileHeader hdr;
        hdr.version = opt.version;
        hdr.windowSize = opt.lz.windowSize;
        hdr.originalSize = inputSize;

        // Only store non-zero frequencies
        for (uint16_t sym = 0; sym < llFreqs.size(); ++sym)
        {
            if (llFreqs[sym] > 0)
            {
                hdr.llFreqs.push_back({sym, llFreqs[sym]});
            }
        }
        for (uint16_t sym = 0; sym < distFreqs.size(); ++sym)
        {
            if (distFreqs[sym] > 0)
            {
                hdr.distFreqs.push_back({sym, distFreqs[sym]});
            }
        }

        // Write header (byte-aligned)
        if (!writeHeader(out, hdr, err))
        {
            return false;
        }

        // Pass 2: Encode tokens with Huffman codes
        BitWriter bw(out);
        for (const auto &t : tokens)
        {
            if (t.kind == TokenKind::Literal)
            {
                if (!llCodec.encode(t.literal, bw))
                {
                    if (err)
                        *err = "deflateStream: failed to encode literal";
                    return false;
                }
            }
            else if (t.kind == TokenKind::Match)
            {
                // Encode match marker (symbol 257)
                if (!llCodec.encode(257, bw))
                {
                    if (err)
                        *err = "deflateStream: failed to encode match marker";
                    return false;
                }

                // Write length as 9 bits (3-258 fits in 9 bits: 0-511)
                bw.writeBits(t.length, 9);

                // Encode distance symbol (always 0 in simplified version)
                if (hasMatches)
                {
                    if (!distCodec.encode(0, bw))
                    {
                        if (err)
                            *err = "deflateStream: failed to encode distance symbol";
                        return false;
                    }
                }

                // Write distance as 15 bits (1-32768 fits in 15 bits: 0-32767)
                bw.writeBits(t.distance, 15);
            }
        }

        // Write EOB
        if (!llCodec.encode(256, bw))
        {
            if (err)
                *err = "deflateStream: failed to encode EOB";
            return false;
        }

        // Flush bit writer
        bw.flush();

        if (!out)
        {
            if (err)
                *err = "deflateStream: output stream error";
            return false;
        }

        return true;
    }

} // namespace fc
