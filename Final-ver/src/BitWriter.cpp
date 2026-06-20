#include "BitWriter.hpp"

BitWriter::BitWriter(std::ostream &outStream) : out(outStream), buffer(0), count(0) {}

void BitWriter::writeBit(bool bit)
{
    buffer = (buffer << 1) | static_cast<unsigned char>(bit);
    if (++count == 8)
    {
        out.put(buffer);
        buffer = 0;
        count = 0;
    }
}
void BitWriter::writeByte(unsigned char byte)
{
    for (int i = 7; i >= 0; --i)
    {
        writeBit((byte >> i) & 1);
    }
}
void BitWriter::writeBits(const std::string &bits)
{
    for (const char &c : bits)
    {
        writeBit(c == '1');
    }
}
void BitWriter::writeBits(
    uint64_t bits,
    uint8_t length)
{
    for (int i = length - 1;
         i >= 0;
         --i)
    {
        writeBit(
            (bits >> i) & 1);
    }
}

void BitWriter::flush()
{
    if (count > 0)
    {
        buffer <<= (8 - count);
        out.put(buffer);
        buffer = 0;
        count = 0;
    }
}