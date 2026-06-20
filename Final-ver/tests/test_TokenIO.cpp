#include "TokenIO.hpp"
#include <cassert>
#include <sstream>
#include <iostream>
int main()
{
    std::vector<Token> tokens = {
        {false, 'a', 0, 0},
        {false, 'b', 0, 0},
        {true, 0, 5, 2}};

    std::stringstream ss;

    writeTokens(tokens, ss);

    auto recovered = readTokens(ss);

    assert(recovered.size() == tokens.size());

    for (size_t i = 0; i < tokens.size(); ++i)
    {

        assert(recovered[i].isMatch == tokens[i].isMatch);

        assert(recovered[i].literal == tokens[i].literal);

        assert(recovered[i].length == tokens[i].length);

        assert(recovered[i].distance == tokens[i].distance);
    }

    return 0;
}