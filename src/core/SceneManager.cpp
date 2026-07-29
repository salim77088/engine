// core/SceneManager.cpp
#include "SceneManager.h"
#include "utils/Logger.h"
#include "utils/FileIO.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace luminus {

// ============================================================================
// Scene
// ============================================================================

Scene::Scene() {
    LM_TRACE("Scene", "Scene created: %s", m_Name.c_str());
}

Scene::~Scene() {
    // Unload textures/models owned by SpriteRenderer/MeshRenderer
    auto spriteView = m_Registry.view<SpriteRenderer>();
    for (auto entity : spriteView) {
        auto& sr = spriteView.get<SpriteRenderer>(entity);
        if (sr.loaded && sr.texture.id != 0) {
            UnloadTexture(sr.texture);
        }
    }
    auto meshView = m_Registry.view<MeshRenderer>();
    for (auto entity : meshView) {
        auto& mr = meshView.get<MeshRenderer>(entity);
        if (mr.loaded && mr.model.materials != nullptr) {
            UnloadModel(mr.model);
        }
    }
    auto audioView = m_Registry.view<AudioSource>();
    for (auto entity : audioView) {
        auto& as = audioView.get<AudioSource>(entity);
        if (as.loaded) {
            if (as.isMusic) UnloadMusicStream(as.music);
            else UnloadSound(as.sound);
        }
    }
    LM_TRACE("Scene", "Scene destroyed: %s", m_Name.c_str());
}

EntityID Scene::CreateEntity(const std::string& name) {
    EntityID e = m_Registry.create();
    m_Registry.emplace<Tag>(e, name.empty() ? "Entity" : name, "Untagged");
    m_Registry.emplace<Transform>(e);
    return e;
}

void Scene::DestroyEntity(EntityID entity) {
    if (m_Registry.valid(entity)) {
        m_Registry.destroy(entity);
    }
}

void Scene::DestroyAllEntities() {
    m_Registry.clear();
}

void Scene::ForEachEntity(std::function<void(EntityID)> fn) {
    // Capture fn by reference and call it for each entity
    auto handler = [&fn](auto entity) { fn(static_cast<EntityID>(entity)); };
    m_Registry.each(handler);
}

bool Scene::Save(const std::string& path) {
    json j;
    j["scene"] = m_Name;
    j["entities"] = json::array();
    
    auto view = m_Registry.view<Tag, Transform>();
    for (auto entity : view) {
        auto& tag = view.get<Tag>(entity);
        auto& tf = view.get<Transform>(entity);
        
        json ej;
        ej["id"] = static_cast<uint32_t>(entity);
        ej["name"] = tag.name;
        ej["tag"] = tag.tag;
        ej["position"] = {tf.position.x, tf.position.y, tf.position.z};
        ej["rotation"] = {tf.rotation.x, tf.rotation.y, tf.rotation.z};
        ej["scale"] = {tf.scale.x, tf.scale.y, tf.scale.z};
        
        if (HasComponent<SpriteRenderer>(entity)) {
            auto& sr = GetComponent<SpriteRenderer>(entity);
            ej["sprite"] = {
                {"texture", sr.texturePath},
                {"tint", {sr.tint.r, sr.tint.g, sr.tint.b, sr.tint.a}},
                {"order", sr.drawOrder}
            };
        }
        if (HasComponent<CameraComponent>(entity)) {
            auto& cam = GetComponent<CameraComponent>(entity);
            ej["camera"] = {
                {"isPrimary", cam.isPrimary},
                {"is2D", cam.is2D},
                {"fov", cam.fov}
            };
        }
        if (HasComponent<ScriptComponent>(entity)) {
            auto& sc = GetComponent<ScriptComponent>(entity);
            ej["script"] = sc.scriptPath;
        }
        
        j["entities"].push_back(ej);
    }
    
    std::string dir = FileIO::GetDirectory(path);
    if (!dir.empty() && !FileIO::Exists(dir)) FileIO::MakeDirs(dir);
    
    bool ok = FileIO::WriteText(path, j.dump(2));
    if (ok) LM_INFO("Scene", "Saved to %s (%d entities)", path.c_str(), (int)j["entities"].size());
    else LM_ERROR("Scene", "Failed to save to %s", path.c_str());
    return ok;
}

bool Scene::Load(const std::string& path) {
    std::string content = FileIO::ReadText(path);
    if (content.empty()) {
        LM_ERROR("Scene", "Cannot read scene file: %s", path.c_str());
        return false;
    }
    
    json j;
    try {
        j = json::parse(content);
    } catch (const std::exception& e) {
        LM_ERROR("Scene", "JSON parse error: %s", e.what());
        return false;
    }
    
    DestroyAllEntities();
    
    if (j.contains("scene")) m_Name = j["scene"];
    
    if (j.contains("entities")) {
        for (auto& ej : j["entities"]) {
            std::string name = ej.value("name", "Entity");
            EntityID e = CreateEntity(name);
            
            auto& tag = GetComponent<Tag>(e);
            tag.tag = ej.value("tag", "Untagged");
            
            auto& tf = GetComponent<Transform>(e);
            if (ej.contains("position")) {
                auto p = ej["position"];
                tf.position = {p[0], p[1], p[2]};
            }
            if (ej.contains("rotation")) {
                auto r = ej["rotation"];
                tf.rotation = {r[0], r[1], r[2]};
            }
            if (ej.contains("scale")) {
                auto s = ej["scale"];
                tf.scale = {s[0], s[1], s[2]};
            }
            
            if (ej.contains("sprite")) {
                SpriteRenderer sr;
                sr.texturePath = ej["sprite"].value("texture", "");
                auto tint = ej["sprite"]["tint"];
                sr.tint = {(unsigned char)tint[0], (unsigned char)tint[1], 
                           (unsigned char)tint[2], (unsigned char)tint[3]};
                sr.drawOrder = ej["sprite"].value("order", 0);
                AddComponent(e, sr);
            }
            
            if (ej.contains("camera")) {
                CameraComponent cc;
                cc.isPrimary = ej["camera"].value("isPrimary", false);
                cc.is2D = ej["camera"].value("is2D", false);
                cc.fov = ej["camera"].value("fov", 60.0f);
                AddComponent(e, cc);
            }
            
            if (ej.contains("script")) {
                ScriptComponent sc;
                sc.scriptPath = ej["script"];
                AddComponent(e, sc);
            }
        }
    }
    
    LM_INFO("Scene", "Loaded %s (%d entities)", path.c_str(), (int)GetEntityCount());
    return true;
}

// ============================================================================
// SceneManager
// ============================================================================

SceneManager& SceneManager::Get() {
    static SceneManager instance;
    return instance;
}

void SceneManager::SetActive(Scene* scene) {
    m_Active = scene;
    if (scene) LM_INFO("SceneManager", "Active scene: %s", scene->GetName().c_str());
}

void SceneManager::PushScene(Scene* scene) {
    m_SceneStack.push_back(std::unique_ptr<Scene>(scene));
    m_Active = scene;
}

void SceneManager::PopScene() {
    if (!m_SceneStack.empty()) {
        m_SceneStack.pop_back();
        m_Active = m_SceneStack.empty() ? nullptr : m_SceneStack.back().get();
    }
}

Scene* SceneManager::NewScene(const std::string& name) {
    auto scene = new Scene();
    scene->SetName(name);
    m_SceneStack.push_back(std::unique_ptr<Scene>(scene));
    m_Active = scene;
    LM_INFO("SceneManager", "Created new scene: %s", name.c_str());
    return scene;
}

} // namespace luminus
