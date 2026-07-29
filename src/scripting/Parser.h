#pragma once
#include <string>
#include <vector>
#include "LuminusScript.h"
#include "Lexer.h"

namespace luminus {

class Parser {
public:
    bool Parse(const std::string& source);
    const std::string& Error() const { return error_; }

    std::vector<ASTNode>&       Ast()       { return ast_; }
    std::vector<size_t>&        StartIdx()  { return startIdx_; }
    std::vector<size_t>&        UpdateIdx() { return updateIdx_; }

private:
    const Token& Peek() const;
    const Token& Consume();
    bool Match(TokenType t);
    bool MatchKw(const std::string& kw);
    void Expect(TokenType t, const char* what);
    void ExpectKw(const std::string& kw);
    void ParseHandler(bool isStart);
    ASTNode ParseStatement();

    std::vector<Token> tokens_;
    size_t             pos_ = 0;
    std::string        error_;
    std::vector<ASTNode> ast_;
    std::vector<size_t>  startIdx_;
    std::vector<size_t>  updateIdx_;
};

} // namespace luminus
