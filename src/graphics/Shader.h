// graphics/Shader.h - Shader management
#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>

namespace luminus {

class ShaderManager {
public:
    static ShaderManager& Get();
    
    Shader LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath);
    Shader GetShader(const std::string& name);
    bool HasShader(const std::string& name);
    
    // Built-in shaders
    void LoadBuiltinShaders();
    Shader GetDefault() { return m_Default; }
    Shader GetPBR() { return m_PBR; }
    Shader GetPostProcess() { return m_PostProcess; }
    
private:
    ShaderManager() = default;
    std::unordered_map<std::string, Shader> m_Shaders;
    Shader m_Default {0};
    Shader m_PBR {0};
    Shader m_PostProcess {0};
};

} // namespace luminus
