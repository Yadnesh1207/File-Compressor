#include "TokenIO.hpp"

void writeTokens(const std::vector<Token> &tokens, std::ostream &out)
{
    uint32_t count = tokens.size();

    out.write(reinterpret_cast<char *>(&count), sizeof(count));

    for (const Token &t : tokens)
    {
        uint8_t type = t.isMatch ? 1 : 0;

        out.write(reinterpret_cast<char *>(&type), sizeof(type));

        if (t.isMatch)
        {
            out.write(reinterpret_cast<const char *>(&t.length), sizeof(t.length));

            out.write(reinterpret_cast<const char *>(&t.distance), sizeof(t.distance));
        }
        else
        {
            out.put(static_cast<char>(t.literal));
        }
    }
}
std::vector<Token> readTokens(std::istream &in)
{
    uint32_t count;

    if (!in.read(reinterpret_cast<char *>(&count), sizeof(count)))
    {
        throw std::runtime_error("Failed to read token count");
    }
    std::cout << "count = " << count << '\n';
    std::vector<Token> tokens;
    if (count > 1000000)
    {
        throw std::runtime_error("Corrupt token count");
    }
    tokens.reserve(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        uint8_t type;

        in.read(reinterpret_cast<char *>(&type), sizeof(type));

        Token t{};

        t.isMatch = (type == 1);

        if (t.isMatch)
        {
            in.read(reinterpret_cast<char *>(&t.length), sizeof(t.length));

            in.read(reinterpret_cast<char *>(&t.distance), sizeof(t.distance));
        }
        else
        {
            char c;

            in.get(c);

            t.literal = static_cast<unsigned char>(c);
        }

        tokens.push_back(t);
    }

    return tokens;
}
std::vector<unsigned char> tokensToBytes(const std::vector<Token> &tokens)
{
    std::stringstream ss;

    writeTokens(tokens, ss);

    std::string s = ss.str();

    return {s.begin(), s.end()};
}
std::vector<Token> bytesToTokens(const std::vector<unsigned char> &data)
{
    std::string s(data.begin(), data.end());

    std::istringstream in(s, std::ios::binary);

    return readTokens(in);
}
std::vector<uint16_t> tokensToSymbols(const std::vector<Token> &tokens)
{
    std::vector<uint16_t> symbols;

    for (const auto &t : tokens)
    {
        if (!t.isMatch)
        {
            symbols.push_back(t.literal);
        }
        else
        {
            symbols.push_back(257 + t.length);

            symbols.push_back(t.distance);
        }
    }

    symbols.push_back(256);

    return symbols;
}
std::vector<Token> symbolsToTokens(const std::vector<uint16_t> &symbols)
{
    std::vector<Token> tokens;

    size_t i = 0;

    while (i < symbols.size())
    {
        uint16_t s = symbols[i];

        if (s == 256)
        {
            break;
        }

        if (s <= 255)
        {
            Token t{};

            t.isMatch = false;
            t.literal = static_cast<unsigned char>(s);

            tokens.push_back(t);

            ++i;
        }
        else
        {
            Token t{};

            t.isMatch = true;

            t.length = s - 257;
            ++i;

            if (i >= symbols.size())
            {
                throw std::runtime_error("Missing distance");
            }

            t.distance = symbols[i];

            tokens.push_back(t);

            ++i;
        }
    }

    return tokens;
}