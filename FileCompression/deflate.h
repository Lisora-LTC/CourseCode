#pragma once
#include <iosfwd>
#include <string>
#include "lz77.h"

namespace fc
{

    struct DeflateOptions
    {
        LZ77Options lz{};
        bool storeFreqTablesInHeader = true;
        uint16_t version = 1;
    };

    // Compress input stream into custom DEFLATE-like container
    bool deflateStream(std::istream &in, std::ostream &out, const DeflateOptions &opt, std::string *err);

} // namespace fc
