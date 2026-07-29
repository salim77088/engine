// graphics/Shader.cpp
#include "Shader.h"
#include "../utils/Logger.h"

namespace luminus {

ShaderManager& ShaderManager::Get() {
    static ShaderManager instance;
    return instance;
}

Shader ShaderManager::LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath) {
    Shader s = ::LoadShader(vsPath.empty() ? nullptr : vsPath.c_str(), 
                            fsPath.empty() ? nullptr : fsPath.c_str());
    m_Shaders[name] = s;
    LM_INFO("Shader", "Loaded '%s' (id=%d, vs=%s, fs=%s)", 
            name.c_str(), s.id, vsPath.c_str(), fsPath.c_str());
    return s;
}

Shader ShaderManager::GetShader(const std::string& name) {
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end()) return it->second;
    return {0};
}

bool ShaderManager::HasShader(const std::string& name) {
    return m_Shaders.find(name) != m_Shaders.end();
}

void ShaderManager::LoadBuiltinShaders() {
    // Use raylib's built-in default shader
    m_Default = ::LoadShader(nullptr, nullptr);
    LM_INFO("Shader", "Built-in default shader loaded (id=%d)", m_Default.id);
    // PBR and post-process would be loaded from files if available
    m_PBR = m_Default;
    m_PostProcess = m_Default;
}

} // namespace luminus
