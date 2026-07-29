#pragma once
// ============================================================================
//  LuminusScript — Luminus Engine's beginner-friendly scripting language
// ============================================================================
//  Design goals:
//    - No boilerplate required (no class, no main, no semicolons needed)
//    - Friendly to non-programmers: keywords are short and obvious
//    - Bilingual identifiers allowed (ASCII + Arabic letters supported in names)
//    - Built-in physics, input, and entity access via simple dot notation
//
//  Example:
//
//      when_start {
//          say "Welcome to Luminus"
//      }
//
//      when_update {
//          if key_held LEFT   { player.x -= 5 }
//          if key_held RIGHT  { player.x += 5 }
//          if key_held UP     { player.y -= 5 }
//          if key_held DOWN   { player.y += 5 }
//          if key_hit SPACE   { player.vy = -10 }
//          player.y += 2
//      }
//
//  Statements:
//    entity.property = value        assignment
//    entity.property += value       compound
//    if key_held KEY { ... }        held check
//    if key_hit KEY { ... }         pressed check
//    say "text"                     log
//    spawn NAME                     create entity
//    destroy NAME                   remove entity
//    load_scene "path"              switch scene
//    exit                           quit game
// ============================================================================

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <variant>

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

enum class NodeKind {
    Assign,        // entity.prop  op  value
    IfKeyHeld,     // if key_held KEY { block }
    IfKeyHit,
    Say,           // say "text"
    Spawn,
    Destroy,
    LoadScene,
    Exit,
    SetBackground,
    SetFps,
};

struct ASTNode {
    NodeKind                 kind;
    std::string              entity;
    std::string              property;
    std::string              op;          // =, +=, -=, *=, /=
    float                    numValue = 0.0f;
    std::string              strValue;
    std::vector<ASTNode>     block;
};

class LuminusScript {
public:
    bool LoadFromFile(const std::string& path);
    bool LoadFromString(const std::string& source);
    const std::string& Error() const { return error_; }

    void RunStartHandlers();
    void RunUpdateHandlers();

    // Register an executor callback for nodes (Engine implements this)
    using Executor = std::function<void(const ASTNode&)>;
    void SetExecutor(Executor cb) { exec_ = std::move(cb); }

    const std::vector<ASTNode>& Ast() const { return ast_; }

private:
    bool Parse();
    bool ParseHandler(const std::vector<Token>& toks, size_t& i);

    std::vector<Token> Tokenize(const std::string& src) const;
    ASTNode            ParseStatement(const std::vector<Token>& toks, size_t& i);
    std::vector<ASTNode> ParseBlock(const std::vector<Token>& toks, size_t& i);

    std::string source_;
    std::string error_;
    std::vector<ASTNode> ast_;
    std::vector<size_t>  startIdx_;
    std::vector<size_t>  updateIdx_;
    Executor             exec_;
};

} // namespace luminus
