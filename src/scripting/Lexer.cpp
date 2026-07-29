#include "Lexer.h"
#include <cctype>
#include <unordered_map>

namespace luminus {

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"when_start",   TokenType::Keyword},
    {"when_update",  TokenType::Keyword},
    {"when_key",     TokenType::Keyword},
    {"if",           TokenType::Keyword},
    {"key_held",     TokenType::Keyword},
    {"key_hit",      TokenType::Keyword},
    {"say",          TokenType::Keyword},
    {"spawn",        TokenType::Keyword},
    {"destroy",      TokenType::Keyword},
    {"load_scene",   TokenType::Keyword},
    {"set_background",TokenType::Keyword},
    {"set_fps",      TokenType::Keyword},
    {"exit",         TokenType::Keyword},
    {"true",         TokenType::Keyword},
    {"false",        TokenType::Keyword},
};

static const std::unordered_map<std::string, TokenType> OPERATORS = {
    {"+=", TokenType::Op}, {"-=", TokenType::Op},
    {"*=", TokenType::Op}, {"/=", TokenType::Op},
    {"=",  TokenType::Op}, {"+", TokenType::Op},
    {"-",  TokenType::Op}, {"*", TokenType::Op},
    {"/",  TokenType::Op},
};

std::vector<Token> Tokenize(const std::string& src) {
    std::vector<Token> out;
    size_t i = 0;
    int line = 1, col = 1;
    while (i < src.size()) {
        char c = src[i];
        if (c == '\n') { line++; col = 1; i++; continue; }
        if (c == ' ' || c == '\t' || c == '\r') { i++; col++; continue; }
        // Comments: # ... \n
        if (c == '#') {
            while (i < src.size() && src[i] != '\n') { i++; col++; }
            continue;
        }
        // Strings: "..."
        if (c == '"') {
            size_t start = ++i; col++;
            std::string s;
            while (i < src.size() && src[i] != '"') {
                if (src[i] == '\\' && i+1 < src.size()) {
                    char n = src[++i];
                    if      (n == 'n') s += '\n';
                    else if (n == 't') s += '\t';
                    else               s += n;
                    col++;
                } else {
                    s += src[i];
                }
                i++; col++;
            }
            if (i < src.size()) { i++; col++; }
            out.push_back({TokenType::String, s, line, col});
            continue;
        }
        // Numbers
        if (std::isdigit((unsigned char)c) || (c == '-' && i+1 < src.size() && std::isdigit((unsigned char)src[i+1]))) {
            std::string n;
            if (c == '-') { n += '-'; i++; col++; }
            while (i < src.size() && (std::isdigit((unsigned char)src[i]) || src[i] == '.')) {
                n += src[i]; i++; col++;
            }
            out.push_back({TokenType::Number, n, line, col});
            continue;
        }
        // Identifiers / keywords (allow Arabic letters too)
        if (std::isalpha((unsigned char)c) || c == '_' || (unsigned char)c >= 0x80) {
            std::string id;
            int startCol = col;
            while (i < src.size() &&
                  (std::isalnum((unsigned char)src[i]) || src[i] == '_' || src[i] == '.' || (unsigned char)src[i] >= 0x80)) {
                id += src[i]; i++; col++;
            }
            auto it = KEYWORDS.find(id);
            if (it != KEYWORDS.end()) {
                out.push_back({TokenType::Keyword, id, line, startCol});
            } else {
                out.push_back({TokenType::Identifier, id, line, startCol});
            }
            continue;
        }
        // Braces, parens, brackets, comma, colon
        if (c == '{') { out.push_back({TokenType::Brace, "{", line, col}); i++; col++; continue; }
        if (c == '}') { out.push_back({TokenType::Brace, "}", line, col}); i++; col++; continue; }
        if (c == '(') { out.push_back({TokenType::Paren, "(", line, col}); i++; col++; continue; }
        if (c == ')') { out.push_back({TokenType::Paren, ")", line, col}); i++; col++; continue; }
        if (c == '[') { out.push_back({TokenType::Bracket, "[", line, col}); i++; col++; continue; }
        if (c == ']') { out.push_back({TokenType::Bracket, "]", line, col}); i++; col++; continue; }
        if (c == ',') { out.push_back({TokenType::Comma, ",", line, col}); i++; col++; continue; }
        if (c == ':') { out.push_back({TokenType::Colon, ":", line, col}); i++; col++; continue; }

        // Multi-char operators
        if (i+1 < src.size()) {
            std::string two = src.substr(i, 2);
            if (OPERATORS.count(two)) {
                out.push_back({TokenType::Op, two, line, col});
                i += 2; col += 2;
                continue;
            }
        }
        if (OPERATORS.count(std::string(1, c))) {
            out.push_back({TokenType::Op, std::string(1, c), line, col});
            i++; col++;
            continue;
        }
        // Unknown char, skip
        i++; col++;
    }
    out.push_back({TokenType::End, "", line, col});
    return out;
}

} // namespace luminus
