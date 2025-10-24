#pragma once
#include <iosfwd>
#include <string>

namespace fc
{

    // Decompress from custom DEFLATE-like container
    bool inflateStream(std::istream &in, std::ostream &out, std::string *err);

} // namespace fc
