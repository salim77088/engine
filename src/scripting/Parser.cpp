#include "Parser.h"
#include "Lexer.h"
#include <sstream>
#include <stdexcept>

namespace luminus {

bool Parser::Parse(const std::string& source) {
    tokens_ = Tokenize(source);
    pos_ = 0;
    ast_.clear();
    startIdx_.clear();
    updateIdx_.clear();
    error_.clear();
    try {
        while (Peek().type != TokenType::End) {
            const Token& t = Peek();
            if (t.type == TokenType::Keyword) {
                if (t.value == "when_start")  { ParseHandler(true);  continue; }
                if (t.value == "when_update") { ParseHandler(false); continue; }
                if (t.value == "when_key")    { ParseHandler(false); continue; }
            }
            // Otherwise treat as top-level statement
            ASTNode n = ParseStatement();
            ast_.push_back(n);
        }
    } catch (const std::exception& e) {
        error_ = e.what();
        return false;
    }
    return true;
}

const Token& Parser::Peek() const {
    return tokens_[std::min(pos_, tokens_.size()-1)];
}

const Token& Parser::Consume() {
    return tokens_[std::min(pos_++, tokens_.size()-1)];
}

bool Parser::Match(TokenType t) {
    return Peek().type == t;
}

bool Parser::MatchKw(const std::string& kw) {
    return Peek().type == TokenType::Keyword && Peek().value == kw;
}

void Parser::Expect(TokenType t, const char* what) {
    if (Peek().type != t) {
        std::ostringstream ss;
        ss << "Line " << Peek().line << ":" << Peek().col
           << " expected " << what << " but got '" << Peek().value << "'";
        throw std::runtime_error(ss.str());
    }
    Consume();
}

void Parser::ExpectKw(const std::string& kw) {
    if (!MatchKw(kw)) {
        std::ostringstream ss;
        ss << "Line " << Peek().line << ":" << Peek().col
           << " expected '" << kw << "' but got '" << Peek().value << "'";
        throw std::runtime_error(ss.str());
    }
    Consume();
}

void Parser::ParseHandler(bool isStart) {
    Consume(); // handler name
    if (isStart) {
        Expect(TokenType::Brace, "'{'");
        while (Peek().type != TokenType::Brace || Peek().value != "}") {
            if (Peek().type == TokenType::End) throw std::runtime_error("Unclosed when_start block");
            ast_.push_back(ParseStatement());
            startIdx_.push_back(ast_.size() - 1);
        }
        Consume(); // }
    } else {
        // when_update or when_key KEY { ... }
        std::string key;
        if (MatchKw("when_key")) {
            // already consumed via caller? No, we already consumed 'when_update' or 'when_key'
            // For when_key, expect a key identifier next
        }
        // Actually we consumed the handler keyword already. Check if next is identifier (when_key case)
        if (Peek().type == TokenType::Identifier) {
            key = Consume().value;
        }
        Expect(TokenType::Brace, "'{'");
        while (Peek().type != TokenType::Brace || Peek().value != "}") {
            if (Peek().type == TokenType::End) throw std::runtime_error("Unclosed block");
            ast_.push_back(ParseStatement());
            updateIdx_.push_back(ast_.size() - 1);
        }
        Consume(); // }
    }
}

ASTNode Parser::ParseStatement() {
    const Token& t = Peek();
    if (t.type == TokenType::Keyword) {
        if (t.value == "say") {
            Consume();
            ASTNode n{NodeKind::Say};
            if (Peek().type == TokenType::String) {
                n.strValue = Consume().value;
            } else {
                throw std::runtime_error("say requires a string argument");
            }
            return n;
        }
        if (t.value == "spawn") {
            Consume();
            ASTNode n{NodeKind::Spawn};
            if (Peek().type == TokenType::Identifier || Peek().type == TokenType::String) {
                n.strValue = Consume().value;
            }
            return n;
        }
        if (t.value == "destroy") {
            Consume();
            ASTNode n{NodeKind::Destroy};
            if (Peek().type == TokenType::Identifier || Peek().type == TokenType::String) {
                n.strValue = Consume().value;
            }
            return n;
        }
        if (t.value == "load_scene") {
            Consume();
            ASTNode n{NodeKind::LoadScene};
            if (Peek().type == TokenType::String) n.strValue = Consume().value;
            return n;
        }
        if (t.value == "set_background") {
            Consume();
            ASTNode n{NodeKind::SetBackground};
            if (Peek().type == TokenType::String || Peek().type == TokenType::Identifier) {
                n.strValue = Consume().value;
            }
            return n;
        }
        if (t.value == "set_fps") {
            Consume();
            ASTNode n{NodeKind::SetFps};
            if (Peek().type == TokenType::Number) n.numValue = std::stof(Consume().value);
            return n;
        }
        if (t.value == "exit") {
            Consume();
            return ASTNode{NodeKind::Exit};
        }
        if (t.value == "if") {
            Consume();
            // Expect key_held or key_hit
            std::string kind = Consume().value;
            std::string key  = Consume().value;
            Expect(TokenType::Brace, "'{'");
            ASTNode n;
            n.kind = (kind == "key_hit") ? NodeKind::IfKeyHit : NodeKind::IfKeyHeld;
            n.strValue = key;
            while (Peek().type != TokenType::Brace || Peek().value != "}") {
                if (Peek().type == TokenType::End) throw std::runtime_error("Unclosed if block");
                n.block.push_back(ParseStatement());
            }
            Consume(); // }
            return n;
        }
    }
    // Otherwise: entity.property op value
    if (t.type != TokenType::Identifier) {
        std::ostringstream ss;
        ss << "Line " << t.line << ":" << t.col << " unexpected token '" << t.value << "'";
        throw std::runtime_error(ss.str());
    }
    ASTNode n{NodeKind::Assign};
    // Identifier may contain dots (entity.prop) due to lexer behavior
    std::string full = Consume().value;
    size_t dot = full.find('.');
    if (dot == std::string::npos) {
        n.entity = "self";
        n.property = full;
    } else {
        n.entity = full.substr(0, dot);
        n.property = full.substr(dot + 1);
    }
    // Op
    if (Peek().type != TokenType::Op) {
        std::ostringstream ss;
        ss << "Line " << Peek().line << " expected operator after " << full;
        throw std::runtime_error(ss.str());
    }
    n.op = Consume().value;
    // Value
    if (Peek().type == TokenType::Number) {
        n.numValue = std::stof(Consume().value);
    } else if (Peek().type == TokenType::String) {
        n.strValue = Consume().value;
    } else if (Peek().type == TokenType::Identifier) {
        n.strValue = Consume().value;
    } else {
        throw std::runtime_error("expected value after operator");
    }
    return n;
}

} // namespace luminus
