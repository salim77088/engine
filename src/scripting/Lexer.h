// scripting/Lexer.h - LuminusScript lexer (tokenizer)
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace luminus::script {

enum class TokenType {
    // Literals
    Number, String, Identifier, True, False, Null,
    
    // Keywords
    Var, Const, Fun, Return, If, Else, While, For, 
    Break, Continue, In, Print, Spawn, Destroy,
    On, Event, Emit, Contract, Verify,
    Entity, Component, Self,
    
    // Operators
    Plus, Minus, Star, Slash, Percent,
    Assign, PlusAssign, MinusAssign, StarAssign, SlashAssign,
    Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual,
    And, Or, Not, BitAnd, BitOr,
    
    // Punctuation
    LeftParen, RightParen, LeftBrace, RightBrace, LeftBracket, RightBracket,
    Comma, Dot, Semicolon, Colon, Arrow, Question,
    
    // Special
    EndOfFile, Unknown,
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token() : type(TokenType::Unknown), line(0), column(0) {}
    Token(TokenType t, const std::string& v, int l, int c) 
        : type(t), value(v), line(l), column(c) {}
};

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> Tokenize();
    
    const std::string& GetError() const { return m_Error; }
    bool HasError() const { return !m_Error.empty(); }

private:
    std::string m_Source;
    size_t m_Pos = 0;
    int m_Line = 1;
    int m_Column = 1;
    std::string m_Error;
    
    char Peek(int offset = 0) const;
    char Advance();
    bool Match(char expected);
    void SkipWhitespaceAndComments();
    
    Token ScanToken();
    Token ScanNumber();
    Token ScanString();
    Token ScanIdentifier();
    
    TokenType GetKeywordType(const std::string& word);
};

} // namespace luminus::script
