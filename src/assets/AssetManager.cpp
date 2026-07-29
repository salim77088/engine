#include "AssetManager.h"
#include "cocos2d.h"
#include <fstream>

namespace luminus {

AssetManager& AssetManager::Instance() { static AssetManager i; return i; }

bool AssetManager::Init() {
    searchPaths_.push_back("assets/");
    searchPaths_.push_back("examples/");
    auto* fs = cocos2d::FileUtils::getInstance();
    if (fs) {
        fs->addSearchPath("assets");
        fs->addSearchPath("examples");
    }
    return true;
}

void AssetManager::Shutdown() { searchPaths_.clear(); }

void AssetManager::AddSearchPath(const std::string& path) {
    searchPaths_.push_back(path);
    auto* fs = cocos2d::FileUtils::getInstance();
    if (fs) fs->addSearchPath(path);
}

std::string AssetManager::Resolve(const std::string& name) {
    auto* fs = cocos2d::FileUtils::getInstance();
    if (fs) return fs->fullPathForFilename(name);
    return name;
}

} // namespace luminus
