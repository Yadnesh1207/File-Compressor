#include "BitReader.hpp"
#include "BitWriter.hpp"
#include "Huffman.hpp"
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
        std::ifstream in(
            inFile,
            std::ios::binary);

        std::ofstream out(
            outFile,
            std::ios::binary);

        out.write("HFZ1", 4);

        uint8_t version = 4;

        out.write(
            reinterpret_cast<char *>(&version),
            sizeof(version));

        uint64_t originalSize = getFileSize(inFile);
        out.write(reinterpret_cast<char *>(&originalSize), sizeof(originalSize));

        uint32_t blockCount = (originalSize + BLOCK_SIZE - 1) / BLOCK_SIZE;

        out.write(reinterpret_cast<char *>(&blockCount), sizeof(blockCount));

        std::vector<unsigned char> block;

        while (readBlock(in, block))
        {
            auto freq = buildFrequencyMap(block);

            std::vector<Node> nodes;

            int root = buildHuffmanTree(freq, nodes);

            auto lengths = buildCodeLengths(root, nodes);

            auto codeMap = buildCanonicalCodes(lengths);

            uint32_t blockSize = block.size();

            std::ostringstream blockData(std::ios::binary);

            BitWriter writer(blockData);

            encodeBlock(block, writer, codeMap);

            writer.flush();
            std::string compressedBits = blockData.str();

            uint32_t compressedSize = compressedBits.size();
            out.write(reinterpret_cast<char *>(&blockSize), sizeof(blockSize));

            out.write(reinterpret_cast<char *>(&compressedSize), sizeof(compressedSize));
            writeCodeLengths(lengths, out);
            out.write(compressedBits.data(), compressedBits.size());
            std::cout
                << "Block size: "
                << blockSize
                << " compressed: "
                << compressedSize
                << '\n';
        }
    }
    else if (mode == "-d")
    {
        std::ifstream in(
            inFile,
            std::ios::binary);

        std::ofstream out(
            outFile,
            std::ios::binary);

        char magic[4];

        in.read(
            magic,
            4);

        if (std::string(magic, 4) != "HFZ1")
        {
            std::cerr
                << "Invalid file\n";

            return 1;
        }

        uint8_t version;

        in.read(
            reinterpret_cast<char *>(
                &version),
            sizeof(version));

        if (version != 4)
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
            uint32_t blockSize;

            in.read(reinterpret_cast<char *>(&blockSize), sizeof(blockSize));
            uint32_t compressedSize;

            in.read(reinterpret_cast<char *>(&compressedSize), sizeof(compressedSize));

            auto lengths = readCodeLengths(in);

            auto codeMap = buildCanonicalCodes(lengths);

            auto decodeTable = buildDecodeTable(codeMap);

            std::vector<char> compressedBytes(compressedSize);

            in.read(compressedBytes.data(), compressedSize);

            std::string blockData(compressedBytes.begin(), compressedBytes.end());

            std::istringstream blockStream(blockData, std::ios::binary);

            BitReader reader(blockStream);
            decodeCanonicalBlock(reader, out, decodeTable, blockSize);
            std::cout
                << "Block "
                << block
                << '\n';

            std::cout
                << "blockSize="
                << blockSize
                << '\n';

            std::cout
                << "compressedSize="
                << compressedSize
                << '\n';

            std::cout
                << "symbols="
                << lengths.size()
                << '\n';
        }
    }
    return 0;
}