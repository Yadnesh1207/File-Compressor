#ifndef TOKEN_IO_HPP
#define TOKEN_IO_HPP

#include "LZ77.hpp"
#include <vector>
#include <iostream>

void writeTokens(const std::vector<Token> &tokens, std::ostream &out);

std::vector<Token> readTokens(std::istream &in);

std::vector<unsigned char> tokensToBytes(const std::vector<Token> &tokens);

std::vector<Token> bytesToTokens(const std::vector<unsigned char> &data);

std::vector<uint16_t> tokensToSymbols(const std::vector<Token> &tokens);

std::vector<Token> symbolsToTokens(const std::vector<uint16_t> &symbols);
#endif