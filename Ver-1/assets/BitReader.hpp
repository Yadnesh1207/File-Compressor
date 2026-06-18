#ifndef BIT_READER_HPP
#define BIT_READER_HPP

#include <istream>

class BitReader {
public:
    BitReader(std::istream& in);
    // returns false when stream ended
    bool readBit(bool& bit);
    bool readByte(unsigned char& byte);

private:
    std::istream& in;
    unsigned char buffer;
    int count;
};

#endif
