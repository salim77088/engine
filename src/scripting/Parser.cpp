// scripting/Parser.cpp
#include "Parser.h"
#include <sstream>

namespace luminus::script {

Parser::Parser(const std::vector<Token>& tokens) : m_Tokens(tokens) {}

const Token& Parser::Peek(int offset) const {
    size_t idx = m_Pos + offset;
    if (idx >= m_Tokens.size()) return m_Tokens.back();
    return m_Tokens[idx];
}

const Token& Parser::Advance() {
    if (m_Pos < m_Tokens.size()) m_Pos++;
    return m_Tokens[m_Pos - 1];
}

bool Parser::Check(TokenType type) const {
    return Peek().type == type;
}

bool Parser::Match(TokenType type) {
    if (!Check(type)) return false;
    Advance();
    return true;
}

bool Parser::MatchAny(std::initializer_list<TokenType> types) {
    for (auto t : types) {
        if (Check(t)) { Advance(); return true; }
    }
    return false;
}

Token Parser::Consume(TokenType type, const std::string& msg) {
    if (Check(type)) return Advance();
    Error(msg, Peek().line);
    return Peek();
}

void Parser::Error(const std::string& msg, int line) {
    if (m_Error.empty()) {
        std::ostringstream oss;
        oss << "Parse error at line " << (line > 0 ? line : Peek().line) << ": " << msg;
        m_Error = oss.str();
    }
}

std::vector<StmtPtr> Parser::Parse() {
    std::vector<StmtPtr> statements;
    while (!Check(TokenType::EndOfFile) && !HasError()) {
        statements.push_back(ParseStatement());
    }
    return statements;
}

StmtPtr Parser::ParseStatement() {
    if (HasError()) return nullptr;
    
    if (Match(TokenType::Var)) return ParseVarDecl(false);
    if (Match(TokenType::Const)) return ParseVarDecl(true);
    if (Match(TokenType::Fun)) return ParseFunDecl();
    if (Match(TokenType::If)) return ParseIf();
    if (Match(TokenType::While)) return ParseWhile();
    if (Match(TokenType::For)) return ParseFor();
    if (Match(TokenType::Return)) return ParseReturn();
    if (Match(TokenType::Print)) return ParsePrint();
    if (Match(TokenType::Spawn)) return ParseSpawn();
    if (Match(TokenType::Destroy)) return ParseDestroy();
    if (Match(TokenType::On)) return ParseOnEvent();
    if (Match(TokenType::Emit)) return ParseEmitEvent();
    if (Match(TokenType::Contract)) return ParseContract();
    if (Match(TokenType::Verify)) return ParseVerify();
    if (Match(TokenType::Component)) return ParseComponentDecl();
    if (Match(TokenType::Break)) {
        Advance();
        Consume(TokenType::Semicolon, "Expected ';' after break");
        auto s = std::make_shared<Stmt>();
        s->type = StmtType::Break;
        return s;
    }
    if (Match(TokenType::Continue)) {
        Advance();
        Consume(TokenType::Semicolon, "Expected ';' after continue");
        auto s = std::make_shared<Stmt>();
        s->type = StmtType::Continue;
        return s;
    }
    if (Match(TokenType::LeftBrace)) return ParseBlock();
    
    // Expression statement
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::ExprStmt;
    s->expr = ParseExpression();
    Consume(TokenType::Semicolon, "Expected ';' after expression");
    return s;
}

StmtPtr Parser::ParseVarDecl(bool isConst) {
    Token name = Consume(TokenType::Identifier, "Expected variable name");
    auto s = std::make_shared<Stmt>();
    s->type = isConst ? StmtType::ConstDecl : StmtType::VarDecl;
    s->name = name.value;
    s->isConst = isConst;
    
    if (Match(TokenType::Assign)) {
        s->expr = ParseExpression();
    }
    Consume(TokenType::Semicolon, "Expected ';' after variable declaration");
    return s;
}

StmtPtr Parser::ParseFunDecl() {
    Token name = Consume(TokenType::Identifier, "Expected function name");
    Consume(TokenType::LeftParen, "Expected '(' after function name");
    
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::FunDecl;
    s->name = name.value;
    
    if (!Check(TokenType::RightParen)) {
        do {
            Token p = Consume(TokenType::Identifier, "Expected parameter name");
            s->params.push_back(p.value);
        } while (Match(TokenType::Comma));
    }
    Consume(TokenType::RightParen, "Expected ')' after parameters");
    
    Consume(TokenType::LeftBrace, "Expected '{' before function body");
    s->body = ParseBlock();
    return s;
}

StmtPtr Parser::ParseIf() {
    Consume(TokenType::LeftParen, "Expected '(' after 'if'");
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::If;
    s->condition = ParseExpression();
    Consume(TokenType::RightParen, "Expected ')' after if condition");
    s->body = ParseStatement();
    if (Match(TokenType::Else)) {
        s->elseBody = ParseStatement();
    }
    return s;
}

StmtPtr Parser::ParseWhile() {
    Consume(TokenType::LeftParen, "Expected '(' after 'while'");
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::While;
    s->condition = ParseExpression();
    Consume(TokenType::RightParen, "Expected ')' after while condition");
    s->body = ParseStatement();
    return s;
}

StmtPtr Parser::ParseFor() {
    Consume(TokenType::LeftParen, "Expected '(' after 'for'");
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::For;
    
    // for (var i in range) or for (init; cond; incr)
    if (Check(TokenType::Var)) {
        Advance();
        Token name = Consume(TokenType::Identifier, "Expected variable name");
        if (Match(TokenType::In)) {
            // for-each style: for (var x in expr) body
            s->name = name.value;
            s->expr = ParseExpression();
            Consume(TokenType::RightParen, "Expected ')'");
            s->body = ParseStatement();
            return s;
        }
        // otherwise treat as var decl initializer
        auto init = std::make_shared<Stmt>();
        init->type = StmtType::VarDecl;
        init->name = name.value;
        if (Match(TokenType::Assign)) init->expr = ParseExpression();
        s->initializer = init;
        Consume(TokenType::Semicolon, "Expected ';'");
    } else if (!Match(TokenType::Semicolon)) {
        auto init = std::make_shared<Stmt>();
        init->type = StmtType::ExprStmt;
        init->expr = ParseExpression();
        s->initializer = init;
        Consume(TokenType::Semicolon, "Expected ';'");
    }
    
    if (!Check(TokenType::Semicolon)) {
        s->condition = ParseExpression();
    }
    Consume(TokenType::Semicolon, "Expected ';'");
    
    if (!Check(TokenType::RightParen)) {
        s->increment = ParseExpression();
    }
    Consume(TokenType::RightParen, "Expected ')'");
    s->body = ParseStatement();
    return s;
}

StmtPtr Parser::ParseBlock() {
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::Block;
    while (!Check(TokenType::RightBrace) && !Check(TokenType::EndOfFile) && !HasError()) {
        s->body_stmts.push_back(ParseStatement());
    }
    Consume(TokenType::RightBrace, "Expected '}' after block");
    return s;
}

StmtPtr Parser::ParseReturn() {
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::Return;
    if (!Check(TokenType::Semicolon)) {
        s->expr = ParseExpression();
    }
    Consume(TokenType::Semicolon, "Expected ';' after return");
    return s;
}

StmtPtr Parser::ParsePrint() {
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::Print;
    s->expr = ParseExpression();
    Consume(TokenType::Semicolon, "Expected ';' after print");
    return s;
}

StmtPtr Parser::ParseSpawn() {
    // spawn "EntityType" at (x, y) [with { ... }]
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::Spawn;
    s->expr = ParseExpression();
    Consume(TokenType::Semicolon, "Expected ';' after spawn");
    return s;
}

StmtPtr Parser::ParseDestroy() {
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::Destroy;
    s->expr = ParseExpression();
    Consume(TokenType::Semicolon, "Expected ';' after destroy");
    return s;
}

StmtPtr Parser::ParseOnEvent() {
    // on "EventName" -> fun body
    // or on EventName(args) { body }
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::OnEvent;
    
    ExprPtr nameExpr = ParseExpression();
    if (nameExpr && nameExpr->type == ExprType::String) {
        s->eventName = nameExpr->stringValue;
    } else if (nameExpr && nameExpr->type == ExprType::Identifier) {
        s->eventName = nameExpr->name;
    }
    
    if (Match(TokenType::Arrow)) {
        // lambda-style
        if (Match(TokenType::LeftBrace)) {
            s->body = ParseBlock();
        } else {
            s->body = ParseStatement();
        }
    } else {
        s->body = ParseStatement();
    }
    return s;
}

StmtPtr Parser::ParseEmitEvent() {
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::EmitEvent;
    s->expr = ParseExpression();
    Consume(TokenType::Semicolon, "Expected ';' after emit");
    return s;
}

StmtPtr Parser::ParseContract() {
    // contract "Name" { rule ... rule ... }
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::Contract;
    
    if (Check(TokenType::String)) {
        s->name = Advance().value;
    } else if (Check(TokenType::Identifier)) {
        s->name = Advance().value;
    }
    
    Consume(TokenType::LeftBrace, "Expected '{' after contract name");
    while (!Check(TokenType::RightBrace) && !Check(TokenType::EndOfFile) && !HasError()) {
        s->rules.push_back(ParseStatement());
    }
    Consume(TokenType::RightBrace, "Expected '}' after contract body");
    return s;
}

StmtPtr Parser::ParseVerify() {
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::Verify;
    s->expr = ParseExpression();
    Consume(TokenType::Semicolon, "Expected ';' after verify");
    return s;
}

StmtPtr Parser::ParseComponentDecl() {
    // component Name { field: type; ... }
    auto s = std::make_shared<Stmt>();
    s->type = StmtType::ComponentDecl;
    Token name = Consume(TokenType::Identifier, "Expected component name");
    s->name = name.value;
    
    if (Match(TokenType::LeftBrace)) {
        while (!Check(TokenType::RightBrace) && !Check(TokenType::EndOfFile) && !HasError()) {
            ParseStatement();  // skip field decls for now
        }
        Consume(TokenType::RightBrace, "Expected '}' after component");
    }
    return s;
}

// ============================================================================
// Expressions
// ============================================================================
ExprPtr Parser::ParseExpression() {
    return ParseAssignment();
}

ExprPtr Parser::ParseAssignment() {
    ExprPtr expr = ParseLogicalOr();
    
    if (Check(TokenType::Assign)) {
        Token op = Advance();
        ExprPtr value = ParseAssignment();
        if (expr && expr->type == ExprType::Identifier) {
            auto e = std::make_shared<Expr>();
            e->type = ExprType::Assignment;
            e->left = expr;
            e->right = value;
            e->line = op.line;
            return e;
        }
        Error("Invalid assignment target", op.line);
    }
    
    if (MatchAny({TokenType::PlusAssign, TokenType::MinusAssign, TokenType::StarAssign, TokenType::SlashAssign})) {
        Token op = m_Tokens[m_Pos - 1];
        ExprPtr value = ParseAssignment();
        auto e = std::make_shared<Expr>();
        e->type = ExprType::CompoundAssignment;
        e->left = expr;
        e->right = value;
        e->op = op.value;
        e->line = op.line;
        return e;
    }
    
    return expr;
}

ExprPtr Parser::ParseLogicalOr() {
    ExprPtr expr = ParseLogicalAnd();
    while (Match(TokenType::Or)) {
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Logical;
        e->left = expr;
        e->op = "or";
        e->right = ParseLogicalAnd();
        expr = e;
    }
    return expr;
}

ExprPtr Parser::ParseLogicalAnd() {
    ExprPtr expr = ParseEquality();
    while (Match(TokenType::And)) {
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Logical;
        e->left = expr;
        e->op = "and";
        e->right = ParseEquality();
        expr = e;
    }
    return expr;
}

ExprPtr Parser::ParseEquality() {
    ExprPtr expr = ParseComparison();
    while (MatchAny({TokenType::Equal, TokenType::NotEqual})) {
        Token op = m_Tokens[m_Pos - 1];
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Binary;
        e->left = expr;
        e->op = op.value;
        e->right = ParseComparison();
        expr = e;
    }
    return expr;
}

ExprPtr Parser::ParseComparison() {
    ExprPtr expr = ParseAddition();
    while (MatchAny({TokenType::Less, TokenType::Greater, TokenType::LessEqual, TokenType::GreaterEqual})) {
        Token op = m_Tokens[m_Pos - 1];
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Binary;
        e->left = expr;
        e->op = op.value;
        e->right = ParseAddition();
        expr = e;
    }
    return expr;
}

ExprPtr Parser::ParseAddition() {
    ExprPtr expr = ParseMultiplication();
    while (MatchAny({TokenType::Plus, TokenType::Minus})) {
        Token op = m_Tokens[m_Pos - 1];
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Binary;
        e->left = expr;
        e->op = op.value;
        e->right = ParseMultiplication();
        expr = e;
    }
    return expr;
}

ExprPtr Parser::ParseMultiplication() {
    ExprPtr expr = ParseUnary();
    while (MatchAny({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
        Token op = m_Tokens[m_Pos - 1];
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Binary;
        e->left = expr;
        e->op = op.value;
        e->right = ParseUnary();
        expr = e;
    }
    return expr;
}

ExprPtr Parser::ParseUnary() {
    if (MatchAny({TokenType::Not, TokenType::Minus})) {
        Token op = m_Tokens[m_Pos - 1];
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Unary;
        e->op = op.value;
        e->right = ParseUnary();
        e->line = op.line;
        return e;
    }
    return ParseCall();
}

ExprPtr Parser::ParseCall() {
    ExprPtr expr = ParsePrimary();
    while (true) {
        if (Match(TokenType::LeftParen)) {
            expr = FinishCall(expr);
        } else if (Match(TokenType::Dot)) {
            Token name = Consume(TokenType::Identifier, "Expected property name after '.'");
            auto e = std::make_shared<Expr>();
            e->type = ExprType::Member;
            e->left = expr;
            e->name = name.value;
            expr = e;
        } else if (Match(TokenType::LeftBracket)) {
            ExprPtr index = ParseExpression();
            Consume(TokenType::RightBracket, "Expected ']'");
            auto e = std::make_shared<Expr>();
            e->type = ExprType::Index;
            e->left = expr;
            e->right = index;
            expr = e;
        } else {
            break;
        }
    }
    return expr;
}

ExprPtr Parser::FinishCall(ExprPtr callee) {
    auto e = std::make_shared<Expr>();
    e->type = ExprType::Call;
    e->callee = callee;
    
    if (!Check(TokenType::RightParen)) {
        do {
            e->args.push_back(ParseExpression());
        } while (Match(TokenType::Comma));
    }
    Consume(TokenType::RightParen, "Expected ')' after arguments");
    return e;
}

ExprPtr Parser::ParsePrimary() {
    if (HasError()) return nullptr;
    
    Token t = Peek();
    
    if (t.type == TokenType::Number) {
        Advance();
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Number;
        e->numberValue = std::stod(t.value);
        e->line = t.line;
        return e;
    }
    if (t.type == TokenType::String) {
        Advance();
        auto e = std::make_shared<Expr>();
        e->type = ExprType::String;
        e->stringValue = t.value;
        e->line = t.line;
        return e;
    }
    if (t.type == TokenType::True) {
        Advance();
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Bool;
        e->boolValue = true;
        return e;
    }
    if (t.type == TokenType::False) {
        Advance();
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Bool;
        e->boolValue = false;
        return e;
    }
    if (t.type == TokenType::Null) {
        Advance();
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Null;
        return e;
    }
    if (t.type == TokenType::Self) {
        Advance();
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Self;
        return e;
    }
    if (t.type == TokenType::Entity) {
        Advance();
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Entity;
        return e;
    }
    if (t.type == TokenType::Identifier) {
        Advance();
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Identifier;
        e->name = t.value;
        e->line = t.line;
        return e;
    }
    if (t.type == TokenType::LeftParen) {
        Advance();
        ExprPtr inner = ParseExpression();
        Consume(TokenType::RightParen, "Expected ')'");
        auto e = std::make_shared<Expr>();
        e->type = ExprType::Grouping;
        e->left = inner;
        return e;
    }
    if (t.type == TokenType::LeftBracket) {
        Advance();
        auto e = std::make_shared<Expr>();
        e->type = ExprType::ListLiteral;
        if (!Check(TokenType::RightBracket)) {
            do {
                e->elements.push_back(ParseExpression());
            } while (Match(TokenType::Comma));
        }
        Consume(TokenType::RightBracket, "Expected ']' after list");
        return e;
    }
    
    Error("Unexpected token '" + t.value + "'", t.line);
    return nullptr;
}

} // namespace luminus::script
