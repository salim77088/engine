// scripting/Parser.h - LuminusScript recursive-descent parser
#pragma once
#include "Lexer.h"
#include <memory>
#include <vector>
#include <variant>

namespace luminus::script {

// Forward declarations
struct Expr;
struct Stmt;

using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;

// ============================================================================
// Expressions
// ============================================================================
enum class ExprType {
    Number, String, Bool, Null, Identifier,
    Binary, Unary, Logical, Assignment, CompoundAssignment,
    Call, Member, Index, Grouping, Self, Entity, ListLiteral,
};

struct Expr {
    ExprType type;
    
    // Literal values
    double numberValue = 0;
    std::string stringValue;
    bool boolValue = false;
    
    // Children
    ExprPtr left;
    ExprPtr right;
    ExprPtr callee;
    std::vector<ExprPtr> args;
    std::string name;        // identifier name or member name
    std::string op;          // operator
    std::vector<ExprPtr> elements;  // for list literal
    
    int line = 0;
    int column = 0;
};

// ============================================================================
// Statements
// ============================================================================
enum class StmtType {
    VarDecl, ConstDecl, FunDecl, Return, If, While, For,
    ExprStmt, Block, Break, Continue, Print,
    Spawn, Destroy, OnEvent, EmitEvent, Contract, Verify,
    ComponentDecl,
};

struct Stmt {
    StmtType type;
    
    std::string name;
    std::vector<std::string> params;
    StmtPtr body;
    StmtPtr elseBody;
    ExprPtr expr;
    ExprPtr condition;
    ExprPtr initializer;
    ExprPtr increment;
    std::vector<StmtPtr> body_stmts;  // for blocks
    
    bool isConst = false;
    bool isExport = false;
    
    // For events
    std::string eventName;
    std::string eventType;
    
    // For contracts
    std::vector<StmtPtr> rules;
    
    int line = 0;
};

// ============================================================================
// Parser
// ============================================================================
class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<StmtPtr> Parse();
    
    const std::string& GetError() const { return m_Error; }
    bool HasError() const { return !m_Error.empty(); }

private:
    std::vector<Token> m_Tokens;
    size_t m_Pos = 0;
    std::string m_Error;
    
    const Token& Peek(int offset = 0) const;
    const Token& Advance();
    bool Check(TokenType type) const;
    bool Match(TokenType type);
    bool MatchAny(std::initializer_list<TokenType> types);
    Token Consume(TokenType type, const std::string& msg);
    
    void Error(const std::string& msg, int line = 0);
    
    // Statements
    StmtPtr ParseStatement();
    StmtPtr ParseVarDecl(bool isConst);
    StmtPtr ParseFunDecl();
    StmtPtr ParseIf();
    StmtPtr ParseWhile();
    StmtPtr ParseFor();
    StmtPtr ParseBlock();
    StmtPtr ParseReturn();
    StmtPtr ParsePrint();
    StmtPtr ParseSpawn();
    StmtPtr ParseDestroy();
    StmtPtr ParseOnEvent();
    StmtPtr ParseEmitEvent();
    StmtPtr ParseContract();
    StmtPtr ParseVerify();
    StmtPtr ParseComponentDecl();
    StmtPtr ParseExprOrDecl();
    
    // Expressions
    ExprPtr ParseExpression();
    ExprPtr ParseAssignment();
    ExprPtr ParseLogicalOr();
    ExprPtr ParseLogicalAnd();
    ExprPtr ParseEquality();
    ExprPtr ParseComparison();
    ExprPtr ParseAddition();
    ExprPtr ParseMultiplication();
    ExprPtr ParseUnary();
    ExprPtr ParseCall();
    ExprPtr ParsePrimary();
    
    ExprPtr FinishCall(ExprPtr callee);
};

} // namespace luminus::script
