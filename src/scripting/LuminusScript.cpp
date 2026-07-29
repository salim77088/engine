#include "LuminusScript.h"
#include "Parser.h"
#include "Lexer.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace luminus {

bool LuminusScript::LoadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        error_ = "Cannot open file: " + path;
        return false;
    }
    std::stringstream ss;
    ss << f.rdbuf();
    source_ = ss.str();
    return LoadFromString(source_);
}

bool LuminusScript::LoadFromString(const std::string& source) {
    source_ = source;
    Parser p;
    if (!p.Parse(source_)) {
        error_ = p.Error();
        return false;
    }
    ast_ = std::move(p.Ast());
    startIdx_ = std::move(p.StartIdx());
    updateIdx_ = std::move(p.UpdateIdx());
    return true;
}

void LuminusScript::RunStartHandlers() {
    if (!exec_) return;
    for (size_t idx : startIdx_) {
        if (idx < ast_.size()) exec_(ast_[idx]);
    }
}

void LuminusScript::RunUpdateHandlers() {
    if (!exec_) return;
    for (size_t idx : updateIdx_) {
        if (idx < ast_.size()) exec_(ast_[idx]);
    }
}

} // namespace luminus
