// scripting/Interpreter.h - LuminusScript tree-walking interpreter
#pragma once
#include "Parser.h"
#include "../core/SceneManager.h"
#include <unordered_map>
#include <string>
#include <variant>
#include <functional>
#include <memory>

namespace luminus::script {

enum class ValueType { Null, Number, String, Bool, Function, Entity, List };

struct Value;
using ValueList = std::vector<Value>;
using NativeFn = std::function<Value(const std::vector<Value>&)>;

struct Function {
    std::string name;
    std::vector<std::string> params;
    StmtPtr body;
    bool isNative = false;
    NativeFn native;
};

struct Value {
    ValueType type = ValueType::Null;
    double number = 0;
    std::string str;
    bool boolean = false;
    std::shared_ptr<Function> func;
    std::shared_ptr<ValueList> list;
    EntityID entity = entt::null;
    
    static Value NullVal() { Value v; v.type = ValueType::Null; return v; }
    static Value Num(double n) { Value v; v.type = ValueType::Number; v.number = n; return v; }
    static Value Str(const std::string& s) { Value v; v.type = ValueType::String; v.str = s; return v; }
    static Value BoolVal(bool b) { Value v; v.type = ValueType::Bool; v.boolean = b; return v; }
    static Value EntityVal(EntityID e) { Value v; v.type = ValueType::Entity; v.entity = e; return v; }
    static Value ListVal() { Value v; v.type = ValueType::List; v.list = std::make_shared<ValueList>(); return v; }
    
    bool IsNull() const { return type == ValueType::Null; }
    bool IsNumber() const { return type == ValueType::Number; }
    bool IsString() const { return type == ValueType::String; }
    bool IsBool() const { return type == ValueType::Bool; }
    bool IsTruthy() const {
        if (type == ValueType::Bool) return boolean;
        if (type == ValueType::Number) return number != 0;
        if (type == ValueType::String) return !str.empty();
        if (type == ValueType::Null) return false;
        return true;
    }
    
    std::string ToString() const;
    bool Equals(const Value& other) const;
};

class Environment {
public:
    Environment() = default;
    explicit Environment(std::shared_ptr<Environment> parent) : m_Parent(parent) {}
    
    void Define(const std::string& name, const Value& value, bool isConst = false);
    Value Get(const std::string& name);
    bool Set(const std::string& name, const Value& value);
    bool Exists(const std::string& name);
    bool IsConst(const std::string& name);
    
    std::shared_ptr<Environment> GetParent() { return m_Parent; }

private:
    struct VarEntry { Value value; bool isConst; };
    std::unordered_map<std::string, VarEntry> m_Vars;
    std::shared_ptr<Environment> m_Parent;
};

class Interpreter {
public:
    Interpreter();
    
    bool Execute(const std::vector<StmtPtr>& program);
    bool ExecuteFile(const std::string& path);
    bool ExecuteString(const std::string& source);
    
    // Call a function defined in the script
    Value CallFunction(const std::string& name, const std::vector<Value>& args);
    
    // Set the entity context for `self` keyword
    void SetEntityContext(EntityID entity) { m_CurrentEntity = entity; }
    EntityID GetEntityContext() const { return m_CurrentEntity; }
    
    // Register native function (for engine bindings)
    void RegisterNative(const std::string& name, NativeFn fn);
    
    // Event system
    void RegisterEvent(const std::string& name, StmtPtr handler);
    void EmitEvent(const std::string& name, const std::vector<Value>& args);
    
    const std::string& GetError() const { return m_Error; }
    bool HasError() const { return !m_Error.empty(); }
    
    std::shared_ptr<Environment> GetGlobal() { return m_Global; }

private:
    std::shared_ptr<Environment> m_Global;
    EntityID m_CurrentEntity = entt::null;
    std::string m_Error;
    
    std::unordered_map<std::string, std::vector<StmtPtr>> m_EventHandlers;
    std::unordered_map<std::string, std::shared_ptr<Function>> m_Functions;
    
    // Control flow signals
    bool m_ShouldReturn = false;
    bool m_ShouldBreak = false;
    bool m_ShouldContinue = false;
    Value m_ReturnValue;
    
    void RuntimeErr(const std::string& msg);
    
    // Statement execution
    void ExecuteStmt(StmtPtr stmt, std::shared_ptr<Environment> env);
    void ExecuteBlock(const std::vector<StmtPtr>& stmts, std::shared_ptr<Environment> env);
    void ExecuteVarDecl(StmtPtr stmt, std::shared_ptr<Environment> env);
    void ExecuteIf(StmtPtr stmt, std::shared_ptr<Environment> env);
    void ExecuteWhile(StmtPtr stmt, std::shared_ptr<Environment> env);
    void ExecuteFor(StmtPtr stmt, std::shared_ptr<Environment> env);
    void ExecuteFunDecl(StmtPtr stmt, std::shared_ptr<Environment> env);
    
    // Expression evaluation
    Value EvalExpr(ExprPtr expr, std::shared_ptr<Environment> env);
    Value EvalBinary(ExprPtr expr, std::shared_ptr<Environment> env);
    Value EvalUnary(ExprPtr expr, std::shared_ptr<Environment> env);
    Value EvalLogical(ExprPtr expr, std::shared_ptr<Environment> env);
    Value EvalAssignment(ExprPtr expr, std::shared_ptr<Environment> env);
    Value EvalCall(ExprPtr expr, std::shared_ptr<Environment> env);
    Value EvalMember(ExprPtr expr, std::shared_ptr<Environment> env);
    
    // Register all built-in native functions
    void RegisterBuiltins();
    
    // Built-in: spawn entity, destroy, etc
    Value BuiltinSpawn(const std::vector<Value>& args);
    Value BuiltinDestroy(const std::vector<Value>& args);
    Value BuiltinPrint(const std::vector<Value>& args);
    Value BuiltinLen(const std::vector<Value>& args);
    Value BuiltinRandom(const std::vector<Value>& args);
    Value BuiltinAbs(const std::vector<Value>& args);
    Value BuiltinSin(const std::vector<Value>& args);
    Value BuiltinCos(const std::vector<Value>& args);
    Value BuiltinFloor(const std::vector<Value>& args);
    Value BuiltinSqrt(const std::vector<Value>& args);
};

} // namespace luminus::script
