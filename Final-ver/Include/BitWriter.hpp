#ifndef BIT_WRITER_HPP
#define BIT_WRITER_HPP

#include <ostream>
#include <string>
#include <cstdint>

class BitWriter {
public:
    BitWriter(std::ostream& out);
    void writeByte(unsigned char byte);
    void writeBit(bool bit);
    void writeBits(const std::string& bits);
    void writeBits(uint64_t bits, uint8_t length);
    void flush();

private:
    std::ostream& out;
    unsigned char buffer;
    int count;
};

#endif
