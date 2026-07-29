#pragma once
#include <string>
#include <unordered_map>

namespace luminus {

class AssetManager {
public:
    static AssetManager& Instance();
    bool Init();
    void Shutdown();
    void AddSearchPath(const std::string& path);
    std::string Resolve(const std::string& name);
private:
    AssetManager() = default;
    std::vector<std::string> searchPaths_;
};

} // namespace luminus
