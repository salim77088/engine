// scripting/Interpreter.cpp
#include "Interpreter.h"
#include "Lexer.h"
#include "Parser.h"
#include "../utils/Logger.h"
#include "../utils/FileIO.h"
#include "../core/Time.h"
#include "../core/Input.h"
#include <cmath>
#include <sstream>

namespace luminus::script {

// ============================================================================
// Value methods
// ============================================================================
std::string Value::ToString() const {
    switch (type) {
        case ValueType::Null: return "null";
        case ValueType::Number: {
            if (number == (int)number) return std::to_string((int)number);
            std::ostringstream oss; oss << number; return oss.str();
        }
        case ValueType::String: return str;
        case ValueType::Bool: return boolean ? "true" : "false";
        case ValueType::Function: return "<fn " + (func ? func->name : "?") + ">";
        case ValueType::Entity: return "<entity:" + std::to_string((uint32_t)entity) + ">";
        case ValueType::List: {
            if (!list) return "[]";
            std::string r = "[";
            for (size_t i = 0; i < list->size(); i++) {
                if (i) r += ", ";
                r += (*list)[i].ToString();
            }
            r += "]";
            return r;
        }
    }
    return "?";
}

bool Value::Equals(const Value& other) const {
    if (type != other.type) return false;
    switch (type) {
        case ValueType::Null: return true;
        case ValueType::Number: return number == other.number;
        case ValueType::String: return str == other.str;
        case ValueType::Bool: return boolean == other.boolean;
        case ValueType::Entity: return entity == other.entity;
        default: return false;
    }
}

// ============================================================================
// Environment
// ============================================================================
void Environment::Define(const std::string& name, const Value& value, bool isConst) {
    m_Vars[name] = {value, isConst};
}

Value Environment::Get(const std::string& name) {
    auto it = m_Vars.find(name);
    if (it != m_Vars.end()) return it->second.value;
    if (m_Parent) return m_Parent->Get(name);
    return Value::NullVal();
}

bool Environment::Set(const std::string& name, const Value& value) {
    auto it = m_Vars.find(name);
    if (it != m_Vars.end()) {
        if (it->second.isConst) return false;
        it->second.value = value;
        return true;
    }
    if (m_Parent) return m_Parent->Set(name, value);
    return false;
}

bool Environment::Exists(const std::string& name) {
    if (m_Vars.find(name) != m_Vars.end()) return true;
    if (m_Parent) return m_Parent->Exists(name);
    return false;
}

bool Environment::IsConst(const std::string& name) {
    auto it = m_Vars.find(name);
    if (it != m_Vars.end()) return it->second.isConst;
    if (m_Parent) return m_Parent->IsConst(name);
    return false;
}

// ============================================================================
// Interpreter
// ============================================================================
Interpreter::Interpreter() {
    m_Global = std::make_shared<Environment>();
    RegisterBuiltins();
}

void Interpreter::RuntimeErr(const std::string& msg) {
    if (m_Error.empty()) {
        m_Error = "Runtime error: " + msg;
        LM_ERROR("Script", "%s", m_Error.c_str());
    }
}

void Interpreter::RegisterNative(const std::string& name, NativeFn fn) {
    auto f = std::make_shared<Function>();
    f->name = name;
    f->isNative = true;
    f->native = fn;
    m_Global->Define(name, Value{ValueType::Function, 0, "", false, f, nullptr, entt::null});
    m_Functions[name] = f;
}

void Interpreter::RegisterBuiltins() {
    RegisterNative("print", [this](const std::vector<Value>& a) -> Value {
        std::string out;
        for (size_t i = 0; i < a.size(); i++) {
            if (i) out += " ";
            out += a[i].ToString();
        }
        LM_INFO("Script", "%s", out.c_str());
        return Value::NullVal();
    });
    RegisterNative("len", [this](const std::vector<Value>& a) -> Value {
        if (a.empty()) return Value::Num(0);
        if (a[0].type == ValueType::String) return Value::Num((double)a[0].str.size());
        if (a[0].type == ValueType::List && a[0].list) return Value::Num((double)a[0].list->size());
        return Value::Num(0);
    });
    RegisterNative("random", [](const std::vector<Value>& a) -> Value {
        if (a.empty()) return Value::Num((double)rand() / RAND_MAX);
        if (a.size() == 1) return Value::Num(rand() % (int)a[0].number);
        return Value::Num(a[0].number + (rand() / (double)RAND_MAX) * (a[1].number - a[0].number));
    });
    RegisterNative("abs", [](const std::vector<Value>& a) -> Value { 
        return a.empty() ? Value::NullVal() : Value::Num(std::abs(a[0].number)); 
    });
    RegisterNative("sin", [](const std::vector<Value>& a) -> Value { 
        return a.empty() ? Value::NullVal() : Value::Num(std::sin(a[0].number)); 
    });
    RegisterNative("cos", [](const std::vector<Value>& a) -> Value { 
        return a.empty() ? Value::NullVal() : Value::Num(std::cos(a[0].number)); 
    });
    RegisterNative("floor", [](const std::vector<Value>& a) -> Value { 
        return a.empty() ? Value::NullVal() : Value::Num(std::floor(a[0].number)); 
    });
    RegisterNative("sqrt", [](const std::vector<Value>& a) -> Value { 
        return a.empty() ? Value::NullVal() : Value::Num(std::sqrt(a[0].number)); 
    });
    RegisterNative("spawn", [this](const std::vector<Value>& a) -> Value {
        return BuiltinSpawn(a);
    });
    RegisterNative("destroy", [this](const std::vector<Value>& a) -> Value {
        return BuiltinDestroy(a);
    });
    
    // Engine bindings
    RegisterNative("deltaTime", [](const std::vector<Value>&) -> Value { 
        return Value::Num(Time::DeltaTime()); 
    });
    RegisterNative("fps", [](const std::vector<Value>&) -> Value { 
        return Value::Num(Time::FPS()); 
    });
    RegisterNative("keyDown", [](const std::vector<Value>& a) -> Value {
        if (a.empty()) return Value::BoolVal(false);
        return Value::BoolVal(IsKeyDown((int)a[0].number));
    });
    RegisterNative("keyPressed", [](const std::vector<Value>& a) -> Value {
        if (a.empty()) return Value::BoolVal(false);
        return Value::BoolVal(IsKeyPressed((int)a[0].number));
    });
    RegisterNative("mouseX", [](const std::vector<Value>&) -> Value { 
        return Value::Num(GetMouseX()); 
    });
    RegisterNative("mouseY", [](const std::vector<Value>&) -> Value { 
        return Value::Num(GetMouseY()); 
    });
    RegisterNative("mouseDown", [](const std::vector<Value>& a) -> Value {
        if (a.empty()) return Value::BoolVal(false);
        return Value::BoolVal(IsMouseButtonDown((int)a[0].number));
    });
}

Value Interpreter::BuiltinSpawn(const std::vector<Value>& args) {
    Scene* scene = SceneManager::Get().GetActive();
    if (!scene || args.empty()) return Value::NullVal();
    
    std::string name = args[0].IsString() ? args[0].str : "Entity";
    EntityID e = scene->CreateEntity(name);
    return Value::EntityVal(e);
}

Value Interpreter::BuiltinDestroy(const std::vector<Value>& args) {
    Scene* scene = SceneManager::Get().GetActive();
    if (!scene || args.empty() || !args[0].IsNumber()) return Value::NullVal();
    EntityID e = (EntityID)(uint32_t)args[0].number;
    scene->DestroyEntity(e);
    return Value::NullVal();
}

bool Interpreter::ExecuteFile(const std::string& path) {
    std::string src = FileIO::ReadText(path);
    if (src.empty()) {
        LM_ERROR("Script", "Cannot read script: %s", path.c_str());
        return false;
    }
    LM_INFO("Script", "Loading script: %s (%d bytes)", path.c_str(), (int)src.size());
    return ExecuteString(src);
}

bool Interpreter::ExecuteString(const std::string& source) {
    Lexer lex(source);
    auto tokens = lex.Tokenize();
    if (lex.HasError()) {
        m_Error = lex.GetError();
        LM_ERROR("Script", "Lex error: %s", m_Error.c_str());
        return false;
    }
    
    Parser parser(tokens);
    auto program = parser.Parse();
    if (parser.HasError()) {
        m_Error = parser.GetError();
        LM_ERROR("Script", "Parse error: %s", m_Error.c_str());
        return false;
    }
    
    return Execute(program);
}

bool Interpreter::Execute(const std::vector<StmtPtr>& program) {
    m_Error.clear();
    m_ShouldReturn = false;
    m_ShouldBreak = false;
    m_ShouldContinue = false;
    
    for (auto& stmt : program) {
        ExecuteStmt(stmt, m_Global);
        if (HasError()) return false;
    }
    return true;
}

Value Interpreter::CallFunction(const std::string& name, const std::vector<Value>& args) {
    auto it = m_Functions.find(name);
    if (it == m_Functions.end()) return Value::NullVal();
    
    auto fn = it->second;
    if (fn->isNative) {
        return fn->native(args);
    }
    
    auto fnEnv = std::make_shared<Environment>(m_Global);
    for (size_t i = 0; i < fn->params.size() && i < args.size(); i++) {
        fnEnv->Define(fn->params[i], args[i]);
    }
    
    m_ShouldReturn = false;
    m_ReturnValue = Value::NullVal();
    
    if (fn->body) {
        ExecuteStmt(fn->body, fnEnv);
    }
    
    return m_ReturnValue;
}

void Interpreter::RegisterEvent(const std::string& name, StmtPtr handler) {
    m_EventHandlers[name].push_back(handler);
}

void Interpreter::EmitEvent(const std::string& name, const std::vector<Value>& args) {
    auto it = m_EventHandlers.find(name);
    if (it == m_EventHandlers.end()) return;
    
    for (auto& handler : it->second) {
        auto env = std::make_shared<Environment>(m_Global);
        // Pass args as $0, $1, ...
        for (size_t i = 0; i < args.size(); i++) {
            env->Define("$" + std::to_string(i), args[i]);
        }
        ExecuteStmt(handler, env);
    }
}

void Interpreter::ExecuteStmt(StmtPtr stmt, std::shared_ptr<Environment> env) {
    if (!stmt || HasError()) return;
    if (m_ShouldReturn || m_ShouldBreak || m_ShouldContinue) return;
    
    switch (stmt->type) {
        case StmtType::VarDecl:
        case StmtType::ConstDecl:
            ExecuteVarDecl(stmt, env);
            break;
        case StmtType::FunDecl:
            ExecuteFunDecl(stmt, env);
            break;
        case StmtType::If:
            ExecuteIf(stmt, env);
            break;
        case StmtType::While:
            ExecuteWhile(stmt, env);
            break;
        case StmtType::For:
            ExecuteFor(stmt, env);
            break;
        case StmtType::Block:
            ExecuteBlock(stmt->body_stmts, std::make_shared<Environment>(env));
            break;
        case StmtType::Return:
            m_ReturnValue = stmt->expr ? EvalExpr(stmt->expr, env) : Value::NullVal();
            m_ShouldReturn = true;
            break;
        case StmtType::Break:
            m_ShouldBreak = true;
            break;
        case StmtType::Continue:
            m_ShouldContinue = true;
            break;
        case StmtType::Print: {
            Value v = stmt->expr ? EvalExpr(stmt->expr, env) : Value::NullVal();
            LM_INFO("Script", "%s", v.ToString().c_str());
            break;
        }
        case StmtType::ExprStmt:
            EvalExpr(stmt->expr, env);
            break;
        case StmtType::Spawn: {
            Value v = EvalExpr(stmt->expr, env);
            BuiltinSpawn({v});
            break;
        }
        case StmtType::Destroy: {
            Value v = EvalExpr(stmt->expr, env);
            BuiltinDestroy({v});
            break;
        }
        case StmtType::OnEvent:
            RegisterEvent(stmt->eventName, stmt->body);
            LM_INFO("Script", "Registered event handler: %s", stmt->eventName.c_str());
            break;
        case StmtType::EmitEvent: {
            Value v = EvalExpr(stmt->expr, env);
            EmitEvent(v.ToString(), {});
            break;
        }
        case StmtType::Contract:
            LM_INFO("Script", "Contract declared: %s (%d rules)", 
                    stmt->name.c_str(), (int)stmt->rules.size());
            // Contracts are evaluated by ContractVM
            break;
        case StmtType::Verify: {
            Value v = EvalExpr(stmt->expr, env);
            if (!v.IsTruthy()) {
                LM_WARN("Script", "Verify failed!");
            }
            break;
        }
        case StmtType::ComponentDecl:
            LM_INFO("Script", "Component declared: %s", stmt->name.c_str());
            break;
    }
}

void Interpreter::ExecuteBlock(const std::vector<StmtPtr>& stmts, std::shared_ptr<Environment> env) {
    for (auto& s : stmts) {
        ExecuteStmt(s, env);
        if (HasError() || m_ShouldReturn || m_ShouldBreak || m_ShouldContinue) return;
    }
}

void Interpreter::ExecuteVarDecl(StmtPtr stmt, std::shared_ptr<Environment> env) {
    Value v = stmt->expr ? EvalExpr(stmt->expr, env) : Value::NullVal();
    env->Define(stmt->name, v, stmt->isConst);
}

void Interpreter::ExecuteFunDecl(StmtPtr stmt, std::shared_ptr<Environment> env) {
    auto fn = std::make_shared<Function>();
    fn->name = stmt->name;
    fn->params = stmt->params;
    fn->body = stmt->body;
    Value fv; fv.type = ValueType::Function; fv.func = fn;
    env->Define(stmt->name, fv);
    m_Functions[stmt->name] = fn;
}

void Interpreter::ExecuteIf(StmtPtr stmt, std::shared_ptr<Environment> env) {
    Value cond = EvalExpr(stmt->condition, env);
    if (cond.IsTruthy()) {
        ExecuteStmt(stmt->body, env);
    } else if (stmt->elseBody) {
        ExecuteStmt(stmt->elseBody, env);
    }
}

void Interpreter::ExecuteWhile(StmtPtr stmt, std::shared_ptr<Environment> env) {
    while (EvalExpr(stmt->condition, env).IsTruthy()) {
        ExecuteStmt(stmt->body, env);
        if (m_ShouldReturn) return;
        if (m_ShouldBreak) { m_ShouldBreak = false; break; }
        if (m_ShouldContinue) { m_ShouldContinue = false; }
    }
}

void Interpreter::ExecuteFor(StmtPtr stmt, std::shared_ptr<Environment> env) {
    auto loopEnv = std::make_shared<Environment>(env);
    
    if (stmt->initializer) {
        ExecuteStmt(stmt->initializer, loopEnv);
    }
    
    while (true) {
        if (stmt->condition) {
            Value cond = EvalExpr(stmt->condition, loopEnv);
            if (!cond.IsTruthy()) break;
        }
        
        // Handle for-each case
        if (!stmt->name.empty() && stmt->expr && !stmt->initializer) {
            Value iterable = EvalExpr(stmt->expr, loopEnv);
            if (iterable.type == ValueType::List && iterable.list) {
                for (auto& item : *iterable.list) {
                    loopEnv->Define(stmt->name, item);
                    ExecuteStmt(stmt->body, loopEnv);
                    if (m_ShouldReturn) return;
                    if (m_ShouldBreak) { m_ShouldBreak = false; break; }
                    if (m_ShouldContinue) { m_ShouldContinue = false; }
                }
                return;
            }
            break;
        }
        
        ExecuteStmt(stmt->body, loopEnv);
        if (m_ShouldReturn) return;
        if (m_ShouldBreak) { m_ShouldBreak = false; break; }
        if (m_ShouldContinue) { m_ShouldContinue = false; }
        
        if (stmt->increment) {
            EvalExpr(stmt->increment, loopEnv);
        }
    }
}

// ============================================================================
// Expressions
// ============================================================================
Value Interpreter::EvalExpr(ExprPtr expr, std::shared_ptr<Environment> env) {
    if (!expr || HasError()) return Value::NullVal();
    
    switch (expr->type) {
        case ExprType::Number: return Value::Num(expr->numberValue);
        case ExprType::String: return Value::Str(expr->stringValue);
        case ExprType::Bool: return Value::BoolVal(expr->boolValue);
        case ExprType::Null: return Value::NullVal();
        case ExprType::Identifier:
            if (expr->name == "self") return Value::EntityVal(m_CurrentEntity);
            return env->Get(expr->name);
        case ExprType::Self: return Value::EntityVal(m_CurrentEntity);
        case ExprType::Entity: return Value::EntityVal(m_CurrentEntity);
        case ExprType::Grouping: return EvalExpr(expr->left, env);
        case ExprType::Binary: return EvalBinary(expr, env);
        case ExprType::Unary: return EvalUnary(expr, env);
        case ExprType::Logical: return EvalLogical(expr, env);
        case ExprType::Assignment:
        case ExprType::CompoundAssignment: return EvalAssignment(expr, env);
        case ExprType::Call: return EvalCall(expr, env);
        case ExprType::Member: return EvalMember(expr, env);
        case ExprType::ListLiteral: {
            Value list = Value::ListVal();
            for (auto& e : expr->elements) {
                list.list->push_back(EvalExpr(e, env));
            }
            return list;
        }
        case ExprType::Index: {
            Value target = EvalExpr(expr->left, env);
            Value idx = EvalExpr(expr->right, env);
            if (target.type == ValueType::List && target.list && idx.IsNumber()) {
                size_t i = (size_t)idx.number;
                if (i < target.list->size()) return (*target.list)[i];
            }
            if (target.type == ValueType::String && idx.IsNumber()) {
                size_t i = (size_t)idx.number;
                if (i < target.str.size()) return Value::Str(std::string(1, target.str[i]));
            }
            return Value::NullVal();
        }
    }
    return Value::NullVal();
}

Value Interpreter::EvalBinary(ExprPtr expr, std::shared_ptr<Environment> env) {
    Value l = EvalExpr(expr->left, env);
    Value r = EvalExpr(expr->right, env);
    
    if (expr->op == "+") {
        if (l.IsString() || r.IsString()) {
            return Value::Str(l.ToString() + r.ToString());
        }
        return Value::Num(l.number + r.number);
    }
    if (expr->op == "-") return Value::Num(l.number - r.number);
    if (expr->op == "*") return Value::Num(l.number * r.number);
    if (expr->op == "/") {
        if (r.number == 0) { RuntimeErr("Division by zero"); return Value::NullVal(); }
        return Value::Num(l.number / r.number);
    }
    if (expr->op == "%") return Value::Num(std::fmod(l.number, r.number));
    if (expr->op == "==") return Value::BoolVal(l.Equals(r));
    if (expr->op == "!=") return Value::BoolVal(!l.Equals(r));
    if (expr->op == "<") return Value::BoolVal(l.number < r.number);
    if (expr->op == ">") return Value::BoolVal(l.number > r.number);
    if (expr->op == "<=") return Value::BoolVal(l.number <= r.number);
    if (expr->op == ">=") return Value::BoolVal(l.number >= r.number);
    if (expr->op == "&") return Value::Num((int)l.number & (int)r.number);
    if (expr->op == "|") return Value::Num((int)l.number | (int)r.number);
    
    return Value::NullVal();
}

Value Interpreter::EvalUnary(ExprPtr expr, std::shared_ptr<Environment> env) {
    Value r = EvalExpr(expr->right, env);
    if (expr->op == "-") return Value::Num(-r.number);
    if (expr->op == "!") return Value::BoolVal(!r.IsTruthy());
    return r;
}

Value Interpreter::EvalLogical(ExprPtr expr, std::shared_ptr<Environment> env) {
    Value l = EvalExpr(expr->left, env);
    if (expr->op == "and") {
        if (!l.IsTruthy()) return l;
        return EvalExpr(expr->right, env);
    }
    if (expr->op == "or") {
        if (l.IsTruthy()) return l;
        return EvalExpr(expr->right, env);
    }
    return Value::NullVal();
}

Value Interpreter::EvalAssignment(ExprPtr expr, std::shared_ptr<Environment> env) {
    if (expr->type == ExprType::Assignment) {
        Value v = EvalExpr(expr->right, env);
        if (expr->left->type == ExprType::Identifier) {
            if (!env->Set(expr->left->name, v)) {
                env->Define(expr->left->name, v);
            }
        }
        return v;
    }
    // Compound assignment
    Value cur = EvalExpr(expr->left, env);
    Value v = EvalExpr(expr->right, env);
    Value result;
    if (expr->op == "+=") result = Value::Num(cur.number + v.number);
    else if (expr->op == "-=") result = Value::Num(cur.number - v.number);
    else if (expr->op == "*=") result = Value::Num(cur.number * v.number);
    else if (expr->op == "/=") {
        if (v.number == 0) { RuntimeErr("Division by zero"); return Value::NullVal(); }
        result = Value::Num(cur.number / v.number);
    }
    if (expr->left->type == ExprType::Identifier) {
        env->Set(expr->left->name, result);
    }
    return result;
}

Value Interpreter::EvalCall(ExprPtr expr, std::shared_ptr<Environment> env) {
    Value callee = EvalExpr(expr->callee, env);
    if (callee.type != ValueType::Function || !callee.func) {
        RuntimeErr("Cannot call non-function");
        return Value::NullVal();
    }
    
    std::vector<Value> args;
    for (auto& a : expr->args) {
        args.push_back(EvalExpr(a, env));
    }
    
    auto fn = callee.func;
    if (fn->isNative) {
        return fn->native(args);
    }
    
    auto fnEnv = std::make_shared<Environment>(m_Global);
    for (size_t i = 0; i < fn->params.size() && i < args.size(); i++) {
        fnEnv->Define(fn->params[i], args[i]);
    }
    
    bool savedReturn = m_ShouldReturn;
    Value savedRetVal = m_ReturnValue;
    m_ShouldReturn = false;
    m_ReturnValue = Value::NullVal();
    
    if (fn->body) {
        ExecuteStmt(fn->body, fnEnv);
    }
    
    Value ret = m_ReturnValue;
    m_ShouldReturn = savedReturn;
    m_ReturnValue = savedRetVal;
    return ret;
}

Value Interpreter::EvalMember(ExprPtr expr, std::shared_ptr<Environment> env) {
    Value target = EvalExpr(expr->left, env);
    
    // Entity member access: entity.position.x, entity.tag, etc.
    if (target.type == ValueType::Entity) {
        Scene* scene = SceneManager::Get().GetActive();
        if (!scene || !scene->GetRegistry().valid(target.entity)) return Value::NullVal();
        
        auto& reg = scene->GetRegistry();
        if (expr->name == "tag" || expr->name == "name") {
            if (reg.all_of<Tag>(target.entity)) {
                return Value::Str(reg.get<Tag>(target.entity).name);
            }
        }
        if (expr->name == "position") {
            if (reg.all_of<Transform>(target.entity)) {
                auto& p = reg.get<Transform>(target.entity).position;
                Value list = Value::ListVal();
                list.list->push_back(Value::Num(p.x));
                list.list->push_back(Value::Num(p.y));
                list.list->push_back(Value::Num(p.z));
                return list;
            }
        }
        if (expr->name == "x") {
            if (reg.all_of<Transform>(target.entity)) return Value::Num(reg.get<Transform>(target.entity).position.x);
        }
        if (expr->name == "y") {
            if (reg.all_of<Transform>(target.entity)) return Value::Num(reg.get<Transform>(target.entity).position.y);
        }
        if (expr->name == "z") {
            if (reg.all_of<Transform>(target.entity)) return Value::Num(reg.get<Transform>(target.entity).position.z);
        }
        if (expr->name == "scale") {
            if (reg.all_of<Transform>(target.entity)) {
                auto& s = reg.get<Transform>(target.entity).scale;
                Value list = Value::ListVal();
                list.list->push_back(Value::Num(s.x));
                list.list->push_back(Value::Num(s.y));
                list.list->push_back(Value::Num(s.z));
                return list;
            }
        }
        if (expr->name == "rotation") {
            if (reg.all_of<Transform>(target.entity)) {
                auto& r = reg.get<Transform>(target.entity).rotation;
                Value list = Value::ListVal();
                list.list->push_back(Value::Num(r.x));
                list.list->push_back(Value::Num(r.y));
                list.list->push_back(Value::Num(r.z));
                return list;
            }
        }
        return Value::NullVal();
    }
    
    // List/string members
    if (target.type == ValueType::List && target.list) {
        if (expr->name == "size" || expr->name == "count") return Value::Num((double)target.list->size());
        if (expr->name == "first" && !target.list->empty()) return target.list->front();
        if (expr->name == "last" && !target.list->empty()) return target.list->back();
    }
    if (target.type == ValueType::String) {
        if (expr->name == "length") return Value::Num((double)target.str.size());
        if (expr->name == "upper") { 
            std::string s = target.str; 
            std::transform(s.begin(), s.end(), s.begin(), ::toupper); 
            return Value::Str(s); 
        }
        if (expr->name == "lower") { 
            std::string s = target.str; 
            std::transform(s.begin(), s.end(), s.begin(), ::tolower); 
            return Value::Str(s); 
        }
    }
    
    return Value::NullVal();
}

} // namespace luminus::script
