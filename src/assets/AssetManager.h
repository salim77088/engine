// assets/AssetManager.h
#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace luminus {

class AssetManager {
public:
    AssetManager();
    ~AssetManager();
    
    Texture2D LoadTexture(const std::string& path);
    Model LoadModel(const std::string& path);
    Shader LoadShader(const std::string& path);
    Font LoadFont(const std::string& path, int size = 16);
    
    void UnloadAll();
    
    // Hot-reload changed assets
    void CheckForChanges();

private:
    struct TexEntry { Texture2D tex; long mtime; };
    std::unordered_map<std::string, TexEntry> m_Textures;
    std::unordered_map<std::string, Model> m_Models;
    std::unordered_map<std::string, Shader> m_Shaders;
    std::unordered_map<std::string, Font> m_Fonts;
};

} // namespace luminus
