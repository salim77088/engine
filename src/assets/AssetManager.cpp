// assets/AssetManager.cpp
#include "AssetManager.h"
#include "../utils/Logger.h"
#include <sys/stat.h>

namespace luminus {

AssetManager::AssetManager() {}
AssetManager::~AssetManager() { UnloadAll(); }

Texture2D AssetManager::LoadTexture(const std::string& path) {
    auto it = m_Textures.find(path);
    if (it != m_Textures.end()) return it->second.tex;
    
    Texture2D t = ::LoadTexture(path.c_str());
    if (t.id == 0) {
        LM_WARN("Assets", "Failed to load texture: %s", path.c_str());
        return t;
    }
    
    struct stat info;
    long mtime = (stat(path.c_str(), &info) == 0) ? info.st_mtime : 0;
    m_Textures[path] = {t, mtime};
    LM_INFO("Assets", "Loaded texture: %s (%dx%d)", path.c_str(), t.width, t.height);
    return t;
}

Model AssetManager::LoadModel(const std::string& path) {
    auto it = m_Models.find(path);
    if (it != m_Models.end()) return it->second;
    
    Model m = ::LoadModel(path.c_str());
    m_Models[path] = m;
    LM_INFO("Assets", "Loaded model: %s", path.c_str());
    return m;
}

Shader AssetManager::LoadShader(const std::string& path) {
    auto it = m_Shaders.find(path);
    if (it != m_Shaders.end()) return it->second;
    
    Shader s = ::LoadShader(0, path.c_str());
    m_Shaders[path] = s;
    LM_INFO("Assets", "Loaded shader: %s", path.c_str());
    return s;
}

Font AssetManager::LoadFont(const std::string& path, int size) {
    std::string key = path + ":" + std::to_string(size);
    auto it = m_Fonts.find(key);
    if (it != m_Fonts.end()) return it->second;
    
    Font f = ::LoadFontEx(path.c_str(), size, nullptr, 0);
    m_Fonts[key] = f;
    LM_INFO("Assets", "Loaded font: %s (size=%d)", path.c_str(), size);
    return f;
}

void AssetManager::UnloadAll() {
    for (auto& p : m_Textures) UnloadTexture(p.second.tex);
    for (auto& p : m_Models) UnloadModel(p.second);
    for (auto& p : m_Shaders) UnloadShader(p.second);
    for (auto& p : m_Fonts) UnloadFont(p.second);
    m_Textures.clear();
    m_Models.clear();
    m_Shaders.clear();
    m_Fonts.clear();
    LM_INFO("Assets", "All assets unloaded");
}

void AssetManager::CheckForChanges() {
    for (auto& p : m_Textures) {
        struct stat info;
        if (stat(p.first.c_str(), &info) == 0 && info.st_mtime > p.second.mtime) {
            UnloadTexture(p.second.tex);
            p.second.tex = ::LoadTexture(p.first.c_str());
            p.second.mtime = info.st_mtime;
            LM_INFO("Assets", "Hot-reloaded: %s", p.first.c_str());
        }
    }
}

} // namespace luminus
