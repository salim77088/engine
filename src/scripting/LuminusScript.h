// scripting/LuminusScript.h - Public API for the scripting system
#pragma once
#include "Interpreter.h"
#include "../core/SceneManager.h"
#include <string>

namespace luminus {

class LuminusScript {
public:
    LuminusScript();
    ~LuminusScript();
    
    bool Init();
    void Shutdown();
    
    // Load and execute a .ls file
    bool LoadFile(const std::string& path);
    bool LoadString(const std::string& source, const std::string& name = "<inline>");
    
    // Call a function defined in the loaded script
    bool CallFunction(const std::string& name);
    bool CallFunction(const std::string& name, EntityID entity);
    bool CallFunction(const std::string& name, EntityID entity, float deltaTime);
    
    // Set entity context for `self` keyword
    void SetEntityContext(EntityID entity) { m_Interpreter.SetEntityContext(entity); }
    
    // Register a native function callable from scripts
    void RegisterNative(const std::string& name, script::NativeFn fn);
    
    // Emit an event to all handlers registered via `on` keyword
    void EmitEvent(const std::string& name);
    void EmitEvent(const std::string& name, const std::vector<script::Value>& args);
    
    // Get underlying interpreter
    script::Interpreter& GetInterpreter() { return m_Interpreter; }
    
    const std::string& GetError() const { return m_Interpreter.GetError(); }
    bool HasError() const { return m_Interpreter.HasError(); }

private:
    script::Interpreter m_Interpreter;
    bool m_Initialized = false;
};

} // namespace luminus
