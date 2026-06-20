#ifndef LZ77_HPP
#define LZ77_HPP

#include <vector>
#include <cstdint>
#include <sstream>
struct Token
{
    bool isMatch;

    unsigned char literal;

    uint16_t length;

    uint16_t distance;
};

std::vector<Token> lz77Compress(const std::vector<unsigned char> &data);

std::vector<unsigned char> lz77Decompress(const std::vector<Token> &tokens);
#endif