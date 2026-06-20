#include "BitReader.hpp"
#include "BitWriter.hpp"
#include "Huffman.hpp"
#include <cassert>
#include <sstream>
#include <iostream>

int main()
{
    std::string data = "this is a test for huffman encoding";
    // build frequency directly from data
    uint64_t originalSize = getFileSize(data);
    std::array<uint64_t, 256> freq{};
    for (unsigned char c : data)
        freq[c]++;

    std::vector<Node> nodes;
    int root = buildHuffmanTree(freq, nodes);
    std::unordered_map<uint16_t, uint8_t> lengths = buildCodeLengths(root, nodes);
    std::unordered_map<uint16_t, HuffCode> codeMap = buildCanonicalCodes(lengths);

    // encode data from memory
    std::istringstream rawIn(data);

    std::ostringstream compressed;

    BitWriter writer(compressed);

    writeCodeLengths(lengths, compressed);

    encodeData(rawIn, writer, codeMap);

    writer.flush();

    std::istringstream inStream(
        compressed.str());

    auto lengths2 =
        readCodeLengths(
            inStream);

    auto codeMap2 =
        buildCanonicalCodes(
            lengths2);

    auto decodeTable =
        buildDecodeTable(
            codeMap2);

    BitReader reader(
        inStream);
    std::ostringstream out;
    decodeCanonical(
        reader,
        out,
        decodeTable,
        data.size());

    if (out.str() != data)
    {
        std::cout
            << "Expected:\n"
            << data
            << "\n\n";
    }

    else
    {
        std::cout
            << "Got:\n"
            << out.str()
            << "\n";
    }
    return 0;
}