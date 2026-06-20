#include "BitReader.hpp"

BitReader::BitReader(std::istream &inStream) : in(inStream), buffer(0), count(0) {}

bool BitReader::readBit(bool &bit)
{
    if (count == 0)
    {
        char ch;
        if (!in.get(ch))
            return false;
        buffer = static_cast<unsigned char>(ch);
        count = 8;
    }
    bit = ((buffer >> 7) & 1);
    buffer <<= 1;
    --count;
    return true;
}
bool BitReader::readByte(unsigned char &byte)
{
    byte = 0;

    for (int i = 0; i < 8; ++i)
    {
        bool bit;

        if (!readBit(bit))
            return false;

        byte = (byte << 1) | bit;
    }

    return true;
}