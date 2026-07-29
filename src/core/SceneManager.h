// core/SceneManager.h - Entity-Component-System scene management
#pragma once
#include "entt/entt.hpp"
#include "raylib.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace luminus {

using EntityID = entt::entity;
const EntityID INVALID_ENTITY = entt::null;

// ============================================================================
// Built-in Components
// ============================================================================

struct Tag {
    std::string name;
    std::string tag;
};

struct Transform {
    Vector3 position {0, 0, 0};
    Vector3 rotation {0, 0, 0};  // Euler degrees
    Vector3 scale {1, 1, 1};
    
    Transform() = default;
    Transform(Vector3 pos) : position(pos) {}
    Transform(Vector3 pos, Vector3 rot, Vector3 scl) : position(pos), rotation(rot), scale(scl) {}
};

struct SpriteRenderer {
    std::string texturePath;
    Texture2D texture {0};
    Color tint {255, 255, 255, 255};
    Rectangle sourceRect {0, 0, 0, 0};  // 0 = use full texture
    Vector2 anchor {0.5f, 0.5f};
    bool flipX = false;
    bool flipY = false;
    int drawOrder = 0;
    bool loaded = false;
};

struct MeshRenderer {
    std::string meshPath;
    Model model {0};
    Color tint {255, 255, 255, 255};
    bool loaded = false;
};

struct CameraComponent {
    Camera3D camera {0};
    bool isPrimary = false;
    bool is2D = false;
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
};

struct RigidBody {
    Vector3 velocity {0, 0, 0};
    Vector3 acceleration {0, 0, 0};
    float mass = 1.0f;
    float drag = 0.0f;
    bool useGravity = true;
    bool isKinematic = false;
    bool isStatic = false;
};

struct Collider {
    enum class Type { Box, Sphere, Circle2D };
    Type type = Type::Box;
    Vector3 size {1, 1, 1};
    float radius = 0.5f;
    bool isTrigger = false;
};

struct ScriptComponent {
    std::string scriptPath;
    bool enabled = true;
    bool initialized = false;
};

struct AudioSource {
    std::string audioPath;
    Sound sound {0};
    Music music {0};
    bool isMusic = false;
    bool loop = false;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool playing = false;
    bool loaded = false;
};

struct Light {
    enum class Type { Directional, Point, Spot };
    Type type = Type::Directional;
    Color color {255, 255, 255, 255};
    float intensity = 1.0f;
    float range = 10.0f;
    Vector3 direction {0, -1, 0};
};

struct Children {
    std::vector<EntityID> children;
    EntityID parent = INVALID_ENTITY;
};

// ============================================================================
// Scene class
// ============================================================================
class Scene {
public:
    Scene();
    ~Scene();
    
    EntityID CreateEntity(const std::string& name = "Entity");
    void DestroyEntity(EntityID entity);
    void DestroyAllEntities();
    
    template<typename... Components>
    auto View() {
        return m_Registry.view<Components...>();
    }
    
    template<typename Component>
    void AddComponent(EntityID entity, const Component& comp = Component{}) {
        m_Registry.emplace_or_replace<Component>(entity, comp);
    }
    
    template<typename Component>
    Component& GetComponent(EntityID entity) {
        return m_Registry.get<Component>(entity);
    }
    
    template<typename Component>
    bool HasComponent(EntityID entity) {
        return m_Registry.all_of<Component>(entity);
    }
    
    template<typename Component>
    void RemoveComponent(EntityID entity) {
        m_Registry.remove<Component>(entity);
    }
    
    entt::registry& GetRegistry() { return m_Registry; }
    const entt::registry& GetRegistry() const { return m_Registry; }
    
    size_t GetEntityCount() const { return m_Registry.size(); }
    
    void SetName(const std::string& name) { m_Name = name; }
    const std::string& GetName() const { return m_Name; }
    
    // Serialization
    bool Save(const std::string& path);
    bool Load(const std::string& path);
    
    // Iteration helpers
    void ForEachEntity(std::function<void(EntityID)> fn);

private:
    entt::registry m_Registry;
    std::string m_Name = "Untitled";
};

// ============================================================================
// SceneManager - manages active scene + scene stack
// ============================================================================
class SceneManager {
public:
    static SceneManager& Get();
    
    void SetActive(Scene* scene);
    Scene* GetActive() { return m_Active; }
    
    void PushScene(Scene* scene);
    void PopScene();
    
    // Helper to create a fresh scene and set it active
    Scene* NewScene(const std::string& name = "Scene");
    
private:
    SceneManager() = default;
    Scene* m_Active = nullptr;
    std::vector<std::unique_ptr<Scene>> m_SceneStack;
};

} // namespace luminus
