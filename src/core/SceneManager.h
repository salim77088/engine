#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace luminus {

struct Entity {
    int         id          = 0;
    std::string name;
    std::string type        = "sprite";   // sprite | text | camera | light | audio
    std::string spritePath;
    float       x           = 0.0f;
    float       y           = 0.0f;
    float       z           = 0.0f;
    float       width       = 50.0f;
    float       height      = 50.0f;
    float       rotation    = 0.0f;
    float       scale       = 1.0f;
    float       opacity     = 1.0f;
    bool        visible     = true;
    std::string color       = "WHITE";
    std::string text;
    int         fontSize    = 24;
    std::string fontPath;
    std::unordered_map<std::string, float>      numbers;
    std::unordered_map<std::string, std::string> strings;
    // Internal cocos2d node handle (set by renderer, not stored as pointer to keep JSON-friendly)
    int         nodeId      = 0;
};

class Scene {
public:
    std::string name;
    std::vector<Entity> entities;
    std::string background = "BLACK";
    bool        physicsEnabled = false;
    float       gravity = 9.8f;
};

class SceneManager {
public:
    static SceneManager& Instance();

    bool LoadFromFile(const std::string& path);
    bool SaveToFile(const std::string& path) const;
    void SetActive(const std::string& name);

    Scene&       GetCurrent()       { return scenes_[currentName_]; }
    const Scene& GetCurrent() const { return scenes_.at(currentName_); }
    const std::string& GetCurrentName() const { return currentName_; }

    void AddEntity(const Entity& e);
    void RemoveEntity(const std::string& name);
    Entity* FindEntity(const std::string& name);

    size_t SceneCount() const { return scenes_.size(); }

private:
    SceneManager() = default;
    std::unordered_map<std::string, Scene> scenes_;
    std::string currentName_;
};

} // namespace luminus
