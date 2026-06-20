#include "Huffman.hpp"
#include "BitReader.hpp"
#include "BitWriter.hpp"
#include <fstream>
#include <queue>
#include <algorithm>

uint64_t getFileSize(const std::string &path)
{
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    return static_cast<uint64_t>(in.tellg());
}
using ArrayFreq = std::array<uint64_t, 256>;
bool readBlock(std::istream &in, std::vector<unsigned char> &block)
{
    block.resize(BLOCK_SIZE);

    in.read(reinterpret_cast<char *>(block.data()), BLOCK_SIZE);

    block.resize(in.gcount());

    return !block.empty();
}
std::array<uint64_t, 256> buildFrequencyMap(const std::vector<unsigned char> &block)
{
    std::array<uint64_t, 256> freq{};

    for (unsigned char c : block)
    {
        ++freq[c];
    }

    return freq;
}
void encodeBlock(const std::vector<unsigned char> &block, BitWriter &writer, const std::unordered_map<uint16_t, HuffCode> &codeMap)
{
    for (unsigned char c : block)
    {
        const HuffCode &code =
            codeMap.at(c);

        writer.writeBits(
            code.bits,
            code.length);
    }
}
struct PQItem
{
    int idx;
    uint64_t freq;
    bool operator>(const PQItem &o) const { return freq > o.freq; }
};
int buildHuffmanTree(const SymbolFreq &freq, std::vector<Node> &nodes)
{
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<>> pq;

    nodes.clear();

    for (const auto &p : freq)
    {
        nodes.push_back(Node{p.first, p.second, -1, -1});

        pq.push({static_cast<int>(nodes.size() - 1), p.second});
    }

    if (pq.empty())
    {
        return -1;
    }

    while (pq.size() > 1)
    {
        auto a = pq.top();
        pq.pop();

        auto b = pq.top();
        pq.pop();

        nodes.push_back(Node{0, a.freq + b.freq, a.idx, b.idx});
        pq.push({static_cast<int>(nodes.size() - 1), a.freq + b.freq});
    }

    return pq.top().idx;
}
int buildHuffmanTree(
    const std::array<uint64_t,256>& freq,
    std::vector<Node>& nodes)
{
    std::priority_queue<
        PQItem,
        std::vector<PQItem>,
        std::greater<>
    > pq;

    nodes.clear();

    for(int i = 0; i < 256; ++i)
    {
        if(freq[i])
        {
            nodes.push_back(
                Node{
                    static_cast<uint16_t>(i),
                    freq[i],
                    -1,
                    -1
                }
            );

            pq.push(
                {
                    static_cast<int>(
                        nodes.size() - 1
                    ),
                    freq[i]
                }
            );
        }
    }

    if(pq.empty())
    {
        return -1;
    }

    while(pq.size() > 1)
    {
        auto a = pq.top();
        pq.pop();

        auto b = pq.top();
        pq.pop();

        nodes.push_back(
            Node{
                0,
                a.freq + b.freq,
                a.idx,
                b.idx
            }
        );

        pq.push(
            {
                static_cast<int>(
                    nodes.size() - 1
                ),
                a.freq + b.freq
            }
        );
    }

    return pq.top().idx;
}
void collectLengths(int idx, const std::vector<Node> &nodes, uint8_t depth, std::unordered_map<uint16_t, uint8_t> &lengths)
{
    const Node &n = nodes[idx];

    if (n.left < 0 && n.right < 0)
    {
        lengths[n.symbol] =
            depth ? depth : 1;

        return;
    }

    collectLengths(
        n.left,
        nodes,
        depth + 1,
        lengths);

    collectLengths(
        n.right,
        nodes,
        depth + 1,
        lengths);
}
std::unordered_map<uint16_t, uint8_t> buildCodeLengths(int root, const std::vector<Node> &nodes)
{
    std::unordered_map<uint16_t, uint8_t>
        lengths;

    collectLengths(
        root,
        nodes,
        0,
        lengths);

    return lengths;
}
struct LengthEntry
{
    uint16_t symbol;
    uint8_t length;
};
std::unordered_map<uint16_t, HuffCode> buildCanonicalCodes(const std::unordered_map<uint16_t, uint8_t> &lengths)
{
    std::vector<LengthEntry>
        entries;

    for (const auto &p : lengths)
    {
        entries.push_back(
            {p.first,
             p.second});
    }

    std::sort(
        entries.begin(),
        entries.end(),
        [](const LengthEntry &a,
           const LengthEntry &b)
        {
            if (a.length != b.length)
                return a.length < b.length;

            return a.symbol < b.symbol;
        });

    std::unordered_map<uint16_t, HuffCode>
        codes;

    uint64_t code = 0;

    uint8_t prevLen =
        entries.front().length;

    for (const auto &e : entries)
    {
        code <<= (e.length - prevLen);

        codes[e.symbol] = {code, e.length};

        ++code;

        prevLen = e.length;
    }

    return codes;
}
std::unordered_map<uint64_t, uint16_t> buildDecodeTable(const std::unordered_map<uint16_t, HuffCode> &codeMap)
{
    std::unordered_map<uint64_t, uint16_t> table;

    for (const auto &p : codeMap)
    {
        const uint16_t symbol = p.first;

        const HuffCode &code = p.second;

        uint64_t key = (code.bits << 8) | code.length;

        table[key] = symbol;
    }

    return table;
}
void writeCodeLengths(const std::unordered_map<uint16_t, uint8_t> &lengths, std::ostream &out)
{
    uint16_t count =
        lengths.size();

    out.write(
        reinterpret_cast<char *>(&count),
        sizeof(count));

    for (const auto &p : lengths)
    {
        uint16_t symbol = p.first;
        uint8_t length = p.second;

        out.write(reinterpret_cast<char *>(&symbol), sizeof(symbol));

        out.write(reinterpret_cast<char *>(&length), sizeof(length));
    }
}

std::unordered_map<uint16_t, uint8_t> readCodeLengths(std::istream &in)
{
    uint16_t count;

    in.read(reinterpret_cast<char *>(&count), sizeof(count));

    std::unordered_map<uint16_t, uint8_t> lengths;

    for (uint16_t i = 0; i < count; ++i)
    {
        uint16_t symbol;
        unsigned char length;

        in.read(reinterpret_cast<char *>(&symbol), sizeof(symbol));

        in.read(reinterpret_cast<char *>(&length), 1);

        lengths[symbol] = length;
    }

    return lengths;
}

void encodeData(
    std::istream &in,
    BitWriter &w,
    const std::unordered_map<uint16_t, HuffCode> &m)
{
    unsigned char c;

    while (in.read(reinterpret_cast<char *>(&c), 1))
    {
        const HuffCode &code = m.at(c);

        w.writeBits(code.bits, code.length);
    }
}
void decodeCanonical(BitReader &reader, std::ostream &out, const std::unordered_map<uint64_t, uint16_t> &decodeTable, uint64_t originalSize)
{
    for (uint64_t written = 0; written < originalSize; ++written)
    {
        uint64_t bits = 0;
        uint8_t length = 0;

        while (true)
        {
            bool bit;

            if (!reader.readBit(bit))
                return;

            bits = (bits << 1) | bit;
            ++length;

            uint64_t key =
                (bits << 8) | length;

            auto it =
                decodeTable.find(key);

            if (it != decodeTable.end())
            {
                out.put(it->second);
                break;
            }
        }
    }
}
void decodeCanonicalBlock(BitReader &reader, std::ostream &out, const std::unordered_map<uint64_t, uint16_t> &decodeTable, uint32_t blockSize)
{
    for (uint32_t written = 0;
         written < blockSize;
         ++written)
    {
        uint64_t bits = 0;
        uint8_t length = 0;

        while (true)
        {
            bool bit;

            if (!reader.readBit(bit))
                return;

            bits =
                (bits << 1) | bit;

            ++length;

            uint64_t key =
                (bits << 8) | length;

            auto it =
                decodeTable.find(key);

            if (it != decodeTable.end())
            {
                out.put(
                    it->second);

                break;
            }
        }
    }
}
SymbolFreq buildFrequencyMap(const std::vector<uint16_t> &symbols)
{
    SymbolFreq freq;

    for (uint16_t s : symbols)
    {
        freq[s]++;
    }

    return freq;
}