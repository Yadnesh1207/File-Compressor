#include "BitReader.hpp"
#include "BitWriter.hpp"
#include "Huffman.hpp"
#include "LZ77.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

constexpr int WINDOW_SIZE = 4096;
constexpr int MIN_MATCH = 3;
constexpr int MAX_MATCH = 258;

uint32_t hash3(const std::vector<unsigned char> &data, size_t pos)
{
    return (static_cast<uint32_t>(data[pos]) << 16) | (static_cast<uint32_t>(data[pos + 1]) << 8) | data[pos + 2];
}
std::vector<Token> lz77Compress(const std::vector<unsigned char> &data)
{
    std::unordered_map<
        uint32_t,
        std::vector<size_t>>
        hashTable;
    std::vector<Token> tokens;

    size_t i = 0;

    size_t literalCount = 0;
    size_t matchCount = 0;

    while (i < data.size())
    {
        uint16_t bestLength = 0;
        uint16_t bestDistance = 0;

        size_t windowStart = (i > WINDOW_SIZE) ? i - WINDOW_SIZE : 0;

        uint32_t key = hash3(data, i);
        auto &candidates = hashTable[key];
        size_t checked = 0;
        for (auto it = candidates.rbegin(); it != candidates.rend(); ++it)
        {

            if (++checked > 256)
            {
                break;
            }
            size_t j = *it;

            if (i - j > WINDOW_SIZE)
            {
                continue;
            }

            uint16_t distance =
                static_cast<uint16_t>(
                    i - j);

            if (distance > WINDOW_SIZE)
            {
                continue;
            }

            uint16_t length = 0;

            while (
                length < MAX_MATCH &&
                i + length < data.size() &&
                data[j + length] ==
                    data[i + length])
            {
                ++length;
            }

            if (length > bestLength)
            {
                bestLength =
                    length;

                bestDistance =
                    distance;
            }
        }
        if (i + 2 < data.size())
        {
            hashTable[hash3(data, i)].push_back(i);
        }

        if (bestLength >= MIN_MATCH)
        {
            Token t;

            t.isMatch = true;
            t.literal = 0;
            t.length = bestLength;
            t.distance = bestDistance;

            tokens.push_back(t);

            ++matchCount;
            for (uint16_t k = 0; k < bestLength; ++k)
            {
                if (i + k + 2 < data.size())
                {
                    hashTable[hash3(data, i + k)].push_back(i + k);
                }
            }
            i += bestLength;
        }
        else
        {
            Token t;

            t.isMatch = false;
            t.literal = data[i];
            t.length = 0;
            t.distance = 0;

            tokens.push_back(t);

            ++literalCount;

            ++i;
        }
    }

    std::cout
        << "Literals: "
        << literalCount
        << '\n';

    std::cout
        << "Matches: "
        << matchCount
        << '\n';

    return tokens;
}
std::vector<unsigned char> lz77Decompress(const std::vector<Token> &tokens)
{
    std::vector<unsigned char>
        output;

    for (const Token &t : tokens)
    {
        if (!t.isMatch)
        {
            output.push_back(
                t.literal);
        }
        else
        {
            if (t.distance > output.size())
            {
                throw std::runtime_error(
                    "Invalid LZ77 distance");
            }

            size_t start =
                output.size() - t.distance;

            for (uint16_t i = 0;
                 i < t.length;
                 ++i)
            {
                output.push_back(
                    output[start + i]);
            }
        }
    }

    return output;
}