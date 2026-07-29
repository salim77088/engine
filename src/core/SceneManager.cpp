#include "SceneManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace luminus {

SceneManager& SceneManager::Instance() {
    static SceneManager inst;
    return inst;
}

bool SceneManager::LoadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "[SceneManager] Cannot open: " << path << "\n";
        return false;
    }
    json j;
    try { f >> j; }
    catch (const std::exception& e) {
        std::cerr << "[SceneManager] Invalid JSON: " << e.what() << "\n";
        return false;
    }
    Scene s;
    s.name = j.value("name", "untitled");
    s.background = j.value("background", "DARKGRAY");
    s.physicsEnabled = j.value("physics", false);
    s.gravity = j.value("gravity", 9.8f);

    if (j.contains("entities")) {
        for (auto& ej : j["entities"]) {
            Entity e;
            e.id         = ej.value("id",         (int)s.entities.size());
            e.name       = ej.value("name",       "entity_" + std::to_string(s.entities.size()));
            e.type       = ej.value("type",       "sprite");
            e.spritePath = ej.value("sprite",     "");
            e.x          = ej.value("x",          0.0f);
            e.y          = ej.value("y",          0.0f);
            e.z          = ej.value("z",          0.0f);
            e.width      = ej.value("w",          50.0f);
            e.height     = ej.value("h",          50.0f);
            e.rotation   = ej.value("rotation",   0.0f);
            e.scale      = ej.value("scale",      1.0f);
            e.opacity    = ej.value("opacity",    1.0f);
            e.visible    = ej.value("visible",    true);
            e.color      = ej.value("color",      "WHITE");
            e.text       = ej.value("text",       "");
            e.fontSize   = ej.value("fontSize",   24);
            e.fontPath   = ej.value("font",       "");
            s.entities.push_back(e);
        }
    }
    scenes_[s.name] = std::move(s);
    currentName_ = j.value("name", "untitled");
    std::cout << "[SceneManager] Loaded '" << currentName_ << "' with "
              << scenes_[currentName_].entities.size() << " entities\n";
    return true;
}

bool SceneManager::SaveToFile(const std::string& path) const {
    auto it = scenes_.find(currentName_);
    if (it == scenes_.end()) return false;
    const Scene& s = it->second;
    json j;
    j["name"] = s.name;
    j["background"] = s.background;
    j["physics"] = s.physicsEnabled;
    j["gravity"] = s.gravity;
    j["entities"] = json::array();
    for (const auto& e : s.entities) {
        j["entities"].push_back({
            {"id", e.id}, {"name", e.name}, {"type", e.type},
            {"sprite", e.spritePath},
            {"x", e.x}, {"y", e.y}, {"z", e.z},
            {"w", e.width}, {"h", e.height},
            {"rotation", e.rotation}, {"scale", e.scale},
            {"opacity", e.opacity}, {"visible", e.visible},
            {"color", e.color}, {"text", e.text},
            {"fontSize", e.fontSize}, {"font", e.fontPath}
        });
    }
    std::ofstream f(path);
    f << j.dump(2);
    return f.good();
}

void SceneManager::SetActive(const std::string& name) {
    if (scenes_.count(name)) currentName_ = name;
}

void SceneManager::AddEntity(const Entity& e) {
    scenes_[currentName_].entities.push_back(e);
}

void SceneManager::RemoveEntity(const std::string& name) {
    auto& v = scenes_[currentName_].entities;
    v.erase(std::remove_if(v.begin(), v.end(),
        [&](const Entity& e){ return e.name == name; }), v.end());
}

Entity* SceneManager::FindEntity(const std::string& name) {
    for (auto& e : scenes_[currentName_].entities) {
        if (e.name == name) return &e;
    }
    return nullptr;
}

} // namespace luminus
