#pragma once
#include <string>
#include <vector>

namespace luminus {

enum class TokenType {
    Identifier, Number, String,
    Keyword, Op, Brace, Paren, Bracket,
    Comma, Colon, Newline, End
};

struct Token {
    TokenType   type;
    std::string value;
    int         line = 0;
    int         col  = 0;
};

std::vector<Token> Tokenize(const std::string& src);

} // namespace luminus
