#include "Huffman.hpp"
#include "BitWriter.hpp"
#include "BitReader.hpp"

#include <cassert>
#include <sstream>
#include <vector>
#include <iostream>

int main()
{
    std::vector<uint16_t> symbols =
        {
            97,
            98,
            262,
            2,
            256};

    auto freq =
        buildFrequencyMap(symbols);

    std::vector<Node> nodes;

    int root =
        buildHuffmanTree(
            freq,
            nodes);

    auto lengths =
        buildCodeLengths(
            root,
            nodes);

    auto codeMap =
        buildCanonicalCodes(
            lengths);

    auto decodeTable =
        buildDecodeTable(
            codeMap);

    std::ostringstream compressed;

    BitWriter writer(
        compressed);

    for (uint16_t s : symbols)
    {
        const HuffCode &code =
            codeMap.at(s);

        writer.writeBits(
            code.bits,
            code.length);
    }

    writer.flush();

    std::istringstream input(
        compressed.str());

    BitReader reader(
        input);

    std::vector<uint16_t>
        recovered;

    for (size_t i = 0; i < symbols.size(); ++i)
    {
        uint64_t bits = 0;
        uint8_t length = 0;

        while (true)
        {
            bool bit;

            if (!reader.readBit(bit))
            {
                assert(false);
            }

            bits =
                (bits << 1) | bit;

            ++length;

            uint64_t key =
                (bits << 8) | length;

            auto it =
                decodeTable.find(key);

            if (it != decodeTable.end())
            {
                recovered.push_back(
                    it->second);

                break;
            }
        }
    }

    assert(
        recovered == symbols);

    std::cout
        << "Symbol Huffman PASS\n";
}