// scripting/LuminusScript.cpp
#include "LuminusScript.h"
#include "../utils/Logger.h"

namespace luminus {

LuminusScript::LuminusScript() {}
LuminusScript::~LuminusScript() { Shutdown(); }

bool LuminusScript::Init() {
    LM_INFO("Script", "LuminusScript v2.0 initialized");
    LM_INFO("Script", "Language features: variables, functions, classes-as-entities, events, contracts");
    m_Initialized = true;
    return true;
}

void LuminusScript::Shutdown() {
    if (m_Initialized) {
        LM_INFO("Script", "LuminusScript shutdown");
        m_Initialized = false;
    }
}

bool LuminusScript::LoadFile(const std::string& path) {
    return m_Interpreter.ExecuteFile(path);
}

bool LuminusScript::LoadString(const std::string& source, const std::string& name) {
    (void)name;
    return m_Interpreter.ExecuteString(source);
}

bool LuminusScript::CallFunction(const std::string& name) {
    if (!m_Initialized) return false;
    m_Interpreter.CallFunction(name, {});
    return !m_Interpreter.HasError();
}

bool LuminusScript::CallFunction(const std::string& name, EntityID entity) {
    if (!m_Initialized) return false;
    m_Interpreter.SetEntityContext(entity);
    m_Interpreter.CallFunction(name, {script::Value::EntityVal(entity)});
    return !m_Interpreter.HasError();
}

bool LuminusScript::CallFunction(const std::string& name, EntityID entity, float dt) {
    if (!m_Initialized) return false;
    m_Interpreter.SetEntityContext(entity);
    m_Interpreter.CallFunction(name, {script::Value::EntityVal(entity), script::Value::Num(dt)});
    return !m_Interpreter.HasError();
}

void LuminusScript::RegisterNative(const std::string& name, script::NativeFn fn) {
    m_Interpreter.RegisterNative(name, fn);
}

void LuminusScript::EmitEvent(const std::string& name) {
    m_Interpreter.EmitEvent(name, {});
}

void LuminusScript::EmitEvent(const std::string& name, const std::vector<script::Value>& args) {
    m_Interpreter.EmitEvent(name, args);
}

} // namespace luminus
