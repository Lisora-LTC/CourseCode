#include "lz77.h"
#include <istream>

namespace fc
{

    bool LZ77Encoder::encode(std::istream &in, std::vector<Token> &outTokens, size_t *inputSize)
    {
        outTokens.clear();
        size_t count = 0;
        while (true)
        {
            int ch = in.get();
            if (ch == EOF)
                break;
            ++count;
            outTokens.push_back(Token::makeLiteral(static_cast<uint8_t>(ch)));
        }
        if (inputSize)
            *inputSize = count;
        return true;
    }

} // namespace fc
