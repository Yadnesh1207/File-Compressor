#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <istream>
#include <ostream>
#include "BitReader.hpp"
#include "BitWriter.hpp"

struct HuffCode
{
    uint64_t bits;
    uint8_t length;
};
struct Node
{
    unsigned char symbol;
    uint64_t freq;
    int left;  // index of left child in nodes vector, -1 if none
    int right; // index of right child, -1 if none
};
bool readBlock(std::istream &in, std::vector<unsigned char> &block);

constexpr size_t BLOCK_SIZE = 64 * 1024;
std::array<uint64_t, 256> buildFrequencyMap(const std::vector<unsigned char> &block);

void encodeBlock(const std::vector<unsigned char> &block, BitWriter &writer, const std::unordered_map<unsigned char, HuffCode> &codeMap);

void decodeCanonicalBlock(BitReader &reader, std::ostream &out, const std::unordered_map<uint64_t, unsigned char> &decodeTable, uint32_t blockSize);
uint64_t getFileSize(const std::string &path);
// builds frequency map from file
std::array<uint64_t, 256> buildFrequencyMap(const std::string &inFile);

// builds Huffman tree nodes and returns root index
int buildHuffmanTree(const std::array<uint64_t, 256> &freq, std::vector<Node> &nodes);

// fills codeMap: symbol -> bitstring

std::unordered_map<unsigned char, uint8_t> buildCodeLengths(int root, const std::vector<Node> &nodes);

std::unordered_map<unsigned char, HuffCode> buildCanonicalCodes(const std::unordered_map<unsigned char, uint8_t> &lengths);
// serialize tree in preorder
void writeCodeLengths(const std::unordered_map<unsigned char, uint8_t> &lengths, std::ostream &out);

std::unordered_map<unsigned char, uint8_t> readCodeLengths(std::istream &in);

void encodeData(std::istream &in, class BitWriter &writer, const std::unordered_map<unsigned char, HuffCode> &codeMap);
std::unordered_map<unsigned char, HuffCode> buildCanonicalCodes(const std::unordered_map<unsigned char, uint8_t> &lengths);

void decodeCanonical(BitReader &reader, std::ostream &out, const std::unordered_map<uint64_t, unsigned char> &decodeTable, uint64_t originalSize);
std::unordered_map<uint64_t, unsigned char> buildDecodeTable(const std::unordered_map<unsigned char, HuffCode> &codeMap);
#endif
