#include "LZ77.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    std::string text = "aaaaabaaaaabaaaaab";

    std::vector<unsigned char> data(text.begin(), text.end());

    auto tokens = lz77Compress(data);

    std::cout << "Input bytes: " << data.size() << '\n';

    std::cout << "Token count: " << tokens.size() << '\n';

    for (const auto &t : tokens)
    {
        if (t.isMatch)
        {
            std::cout << "MATCH " << "len=" << t.length << " dist=" << t.distance << '\n';
        }
        else
        {
            std::cout << "LITERAL " << static_cast<char>(t.literal) << '\n';
        }
    }
    assert(tokens.size() < data.size());
    auto reconstructed = lz77Decompress(tokens);

    std::string result(reconstructed.begin(), reconstructed.end());
    if (result == text)
    {
        std::cout
            << "LZ77 Round Trip: PASS\n";
    }
    else
    {
        std::cout
            << "LZ77 Round Trip: FAIL\n";
    }
    assert(result == text);
    assert(
        tokens.size() < data.size());

    std::cout
        << "Compression Check: PASS\n";
    return 0;
}