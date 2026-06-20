#include "BitReader.hpp"
#include "BitWriter.hpp"
#include "Huffman.hpp"
#include "TokenIO.hpp"
#include "LZ77.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: ./zipper -c|-d <input> <output>\n";
        return 1;
    }
    std::string mode = argv[1];
    std::string inFile = argv[2];
    std::string outFile = argv[3];

    if (mode == "-c")
    {
        std::ifstream in(inFile, std::ios::binary);

        std::ofstream out(outFile, std::ios::binary);

        out.write("HFZ1", 4);

        uint8_t version = 5;

        out.write(reinterpret_cast<char *>(&version), sizeof(version));

        uint64_t originalSize = getFileSize(inFile);
        out.write(reinterpret_cast<char *>(&originalSize), sizeof(originalSize));

        uint32_t blockCount = (originalSize + BLOCK_SIZE - 1) / BLOCK_SIZE;

        out.write(reinterpret_cast<char *>(&blockCount), sizeof(blockCount));

        std::vector<unsigned char> block;

        while (readBlock(in, block))
        {
            auto tokens = lz77Compress(block);

            auto symbols = tokensToSymbols(tokens);

            auto freq = buildFrequencyMap(symbols);

            std::vector<Node> nodes;

            int root = buildHuffmanTree(freq, nodes);

            auto lengths = buildCodeLengths(root, nodes);

            auto codeMap = buildCanonicalCodes(lengths);
            uint32_t originalBlockSize = block.size();
            uint32_t blockSize = block.size();
            uint32_t symbolCount = symbols.size();

            std::ostringstream blockData(std::ios::binary);

            BitWriter writer(blockData);

            for (uint16_t symbol : symbols)
            {
                const HuffCode &code = codeMap.at(symbol);

                writer.writeBits(code.bits, code.length);
            }
            writer.flush();
            std::string compressedBits = blockData.str();

            uint32_t compressedSize = compressedBits.size();
            out.write(reinterpret_cast<char *>(&originalBlockSize), sizeof(originalBlockSize));

            out.write(reinterpret_cast<char *>(&symbolCount), sizeof(symbolCount));
            out.write(reinterpret_cast<char *>(&compressedSize), sizeof(compressedSize));
            writeCodeLengths(lengths, out);
            out.write(compressedBits.data(), compressedBits.size());
            std::cout << "Block size: " << blockSize << " compressed: " << compressedSize << '\n';
        }
    }
    else if (mode == "-d")
    {
        std::ifstream in(inFile, std::ios::binary);

        std::ofstream out(outFile, std::ios::binary);

        char magic[4];

        in.read(magic, 4);

        if (std::string(magic, 4) != "HFZ1")
        {
            std::cerr << "Invalid file\n";

            return 1;
        }

        uint8_t version;

        in.read(
            reinterpret_cast<char *>(
                &version),
            sizeof(version));

        if (version != 5)
        {
            std::cerr
                << "Unsupported version\n";

            return 1;
        }
        uint64_t originalSize;

        in.read(reinterpret_cast<char *>(&originalSize), sizeof(originalSize));

        uint32_t blockCount;

        in.read(reinterpret_cast<char *>(&blockCount), sizeof(blockCount));

        for (uint32_t block = 0; block < blockCount; ++block)
        {
            uint32_t originalBlockSize;

            in.read(reinterpret_cast<char *>(&originalBlockSize), sizeof(originalBlockSize));

            uint32_t symbolCount;

            in.read(reinterpret_cast<char *>(&symbolCount), sizeof(symbolCount));

            uint32_t compressedSize;

            in.read(reinterpret_cast<char *>(&compressedSize), sizeof(compressedSize));

            auto lengths = readCodeLengths(in);

            auto codeMap = buildCanonicalCodes(lengths);

            auto decodeTable = buildDecodeTable(codeMap);

            std::vector<char> compressedBytes(compressedSize);

            in.read(compressedBytes.data(), compressedSize);

            std::string compressedBlockData(compressedBytes.begin(), compressedBytes.end());

            std::istringstream blockStream(compressedBlockData, std::ios::binary);

            BitReader reader(blockStream);
            std::vector<uint16_t> symbols;
            for (uint32_t i = 0; i < symbolCount; ++i)
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

                    bits = (bits << 1) | bit;

                    ++length;

                    uint64_t key = (bits << 8) | length;

                    auto it = decodeTable.find(key);

                    if (it != decodeTable.end())
                    {
                        symbols.push_back(it->second);

                        break;
                    }
                }
            }

            auto tokens = symbolsToTokens(symbols);
            auto blockData = lz77Decompress(tokens);
            for (unsigned char c : blockData)
            {
                out.put(c);
            }
        }
    }
    return 0;
}