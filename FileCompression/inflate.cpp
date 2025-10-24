#include "inflate.h"
#include "bit_io.h"
#include "huffman.h"
#include <string>

namespace fc
{

    bool inflateStream(std::istream & /*in*/, std::ostream & /*out*/, std::string *err)
    {
        if (err)
            *err = "inflateStream: not implemented (skeleton)";
        return false;
    }

} // namespace fc
