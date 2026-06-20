#include "TokenIO.hpp"

#include <cassert>
#include <iostream>

int main()
{
    std::vector<Token> tokens = {{false, 'a', 0, 0}, {false, 'b', 0, 0}, {true, 0, 5, 2}};

    auto symbols = tokensToSymbols(tokens);

    auto recovered = symbolsToTokens(symbols);

    assert(recovered.size() == tokens.size());

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        assert(recovered[i].isMatch == tokens[i].isMatch);

        assert(recovered[i].literal == tokens[i].literal);

        assert(recovered[i].length == tokens[i].length);

        assert(recovered[i].distance == tokens[i].distance);
    }

    std::cout << "Symbol round-trip PASS\n";
}