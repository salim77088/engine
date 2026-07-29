// scripting/Lexer.cpp
#include "Lexer.h"
#include <cctype>
#include <cstring>

namespace luminus::script {

Lexer::Lexer(const std::string& source) : m_Source(source) {}

char Lexer::Peek(int offset) const {
    size_t idx = m_Pos + offset;
    if (idx >= m_Source.size()) return '\0';
    return m_Source[idx];
}

char Lexer::Advance() {
    if (m_Pos >= m_Source.size()) return '\0';
    char c = m_Source[m_Pos++];
    if (c == '\n') { m_Line++; m_Column = 1; }
    else { m_Column++; }
    return c;
}

bool Lexer::Match(char expected) {
    if (Peek() != expected) return false;
    Advance();
    return true;
}

void Lexer::SkipWhitespaceAndComments() {
    while (m_Pos < m_Source.size()) {
        char c = Peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            Advance();
        } else if (c == '/' && Peek(1) == '/') {
            while (m_Pos < m_Source.size() && Peek() != '\n') Advance();
        } else if (c == '/' && Peek(1) == '*') {
            Advance(); Advance();
            while (m_Pos < m_Source.size() && !(Peek() == '*' && Peek(1) == '/')) Advance();
            if (m_Pos < m_Source.size()) { Advance(); Advance(); }
        } else {
            break;
        }
    }
}

TokenType Lexer::GetKeywordType(const std::string& word) {
    if (word == "var") return TokenType::Var;
    if (word == "const") return TokenType::Const;
    if (word == "fun") return TokenType::Fun;
    if (word == "return") return TokenType::Return;
    if (word == "if") return TokenType::If;
    if (word == "else") return TokenType::Else;
    if (word == "while") return TokenType::While;
    if (word == "for") return TokenType::For;
    if (word == "break") return TokenType::Break;
    if (word == "continue") return TokenType::Continue;
    if (word == "in") return TokenType::In;
    if (word == "print") return TokenType::Print;
    if (word == "spawn") return TokenType::Spawn;
    if (word == "destroy") return TokenType::Destroy;
    if (word == "on") return TokenType::On;
    if (word == "event") return TokenType::Event;
    if (word == "emit") return TokenType::Emit;
    if (word == "contract") return TokenType::Contract;
    if (word == "verify") return TokenType::Verify;
    if (word == "entity") return TokenType::Entity;
    if (word == "component") return TokenType::Component;
    if (word == "self") return TokenType::Self;
    if (word == "true") return TokenType::True;
    if (word == "false") return TokenType::False;
    if (word == "null") return TokenType::Null;
    return TokenType::Identifier;
}

Token Lexer::ScanNumber() {
    int startLine = m_Line, startCol = m_Column;
    std::string num;
    while (m_Pos < m_Source.size() && (std::isdigit(Peek()) || Peek() == '.')) {
        num += Advance();
    }
    return Token(TokenType::Number, num, startLine, startCol);
}

Token Lexer::ScanString() {
    int startLine = m_Line, startCol = m_Column;
    Advance();  // skip opening quote
    std::string str;
    while (m_Pos < m_Source.size() && Peek() != '"') {
        char c = Advance();
        if (c == '\\' && m_Pos < m_Source.size()) {
            char next = Advance();
            switch (next) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '"': str += '"'; break;
                case '\\': str += '\\'; break;
                default: str += '\\'; str += next; break;
            }
        } else {
            str += c;
        }
    }
    if (m_Pos >= m_Source.size()) {
        m_Error = "Unterminated string at line " + std::to_string(startLine);
        return Token(TokenType::Unknown, str, startLine, startCol);
    }
    Advance();  // skip closing quote
    return Token(TokenType::String, str, startLine, startCol);
}

Token Lexer::ScanIdentifier() {
    int startLine = m_Line, startCol = m_Column;
    std::string id;
    while (m_Pos < m_Source.size() && (std::isalnum(Peek()) || Peek() == '_')) {
        id += Advance();
    }
    return Token(GetKeywordType(id), id, startLine, startCol);
}

Token Lexer::ScanToken() {
    SkipWhitespaceAndComments();
    if (m_Pos >= m_Source.size()) return Token(TokenType::EndOfFile, "EOF", m_Line, m_Column);
    
    char c = Peek();
    int line = m_Line, col = m_Column;
    
    if (std::isdigit(c)) return ScanNumber();
    if (c == '"') return ScanString();
    if (std::isalpha(c) || c == '_') return ScanIdentifier();
    
    Advance();
    switch (c) {
        case '+': 
            if (Match('=')) return Token(TokenType::PlusAssign, "+=", line, col);
            return Token(TokenType::Plus, "+", line, col);
        case '-':
            if (Match('=')) return Token(TokenType::MinusAssign, "-=", line, col);
            if (Match('>')) return Token(TokenType::Arrow, "->", line, col);
            return Token(TokenType::Minus, "-", line, col);
        case '*':
            if (Match('=')) return Token(TokenType::StarAssign, "*=", line, col);
            return Token(TokenType::Star, "*", line, col);
        case '/':
            if (Match('=')) return Token(TokenType::SlashAssign, "/=", line, col);
            return Token(TokenType::Slash, "/", line, col);
        case '%':
            return Token(TokenType::Percent, "%", line, col);
        case '=':
            if (Match('=')) return Token(TokenType::Equal, "==", line, col);
            return Token(TokenType::Assign, "=", line, col);
        case '!':
            if (Match('=')) return Token(TokenType::NotEqual, "!=", line, col);
            return Token(TokenType::Not, "!", line, col);
        case '<':
            if (Match('=')) return Token(TokenType::LessEqual, "<=", line, col);
            return Token(TokenType::Less, "<", line, col);
        case '>':
            if (Match('=')) return Token(TokenType::GreaterEqual, ">=", line, col);
            return Token(TokenType::Greater, ">", line, col);
        case '&':
            if (Match('&')) return Token(TokenType::And, "&&", line, col);
            return Token(TokenType::BitAnd, "&", line, col);
        case '|':
            if (Match('|')) return Token(TokenType::Or, "||", line, col);
            return Token(TokenType::BitOr, "|", line, col);
        case '(':
            return Token(TokenType::LeftParen, "(", line, col);
        case ')':
            return Token(TokenType::RightParen, ")", line, col);
        case '{':
            return Token(TokenType::LeftBrace, "{", line, col);
        case '}':
            return Token(TokenType::RightBrace, "}", line, col);
        case '[':
            return Token(TokenType::LeftBracket, "[", line, col);
        case ']':
            return Token(TokenType::RightBracket, "]", line, col);
        case ',':
            return Token(TokenType::Comma, ",", line, col);
        case '.':
            return Token(TokenType::Dot, ".", line, col);
        case ';':
            return Token(TokenType::Semicolon, ";", line, col);
        case ':':
            return Token(TokenType::Colon, ":", line, col);
        case '?':
            return Token(TokenType::Question, "?", line, col);
        default:
            m_Error = "Unexpected character '" + std::string(1, c) + "' at line " + std::to_string(line);
            return Token(TokenType::Unknown, std::string(1, c), line, col);
    }
}

std::vector<Token> Lexer::Tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token t = ScanToken();
        tokens.push_back(t);
        if (t.type == TokenType::EndOfFile) break;
        if (HasError()) break;
    }
    return tokens;
}

} // namespace luminus::script
