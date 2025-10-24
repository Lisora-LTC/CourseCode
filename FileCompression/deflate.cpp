#include "deflate.h"
#include "bit_io.h"
#include "huffman.h"
#include <vector>
#include <string>

namespace fc
{

    bool deflateStream(std::istream &in, std::ostream & /*out*/, const DeflateOptions & /*opt*/, std::string *err)
    {
        // Skeleton only: not implemented yet
        if (err)
            *err = "deflateStream: not implemented (skeleton)";
        return false;
    }

} // namespace fc
