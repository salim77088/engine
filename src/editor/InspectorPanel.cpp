// editor/InspectorPanel.cpp
#include "InspectorPanel.h"
#include "../core/SceneManager.h"
#include "../utils/Logger.h"
#include <cstdio>

namespace luminus {

void InspectorPanel::Init() {}
void InspectorPanel::Update() {}

void InspectorPanel::Render(Rectangle bounds, uint32_t selectedEntity) {
    DrawRectangleRec(bounds, Color{35, 35, 40, 255});
    DrawText("INSPECTOR", (int)bounds.x + 10, (int)bounds.y + 6, 14, Color{200, 200, 210, 255});
    DrawLine(bounds.x, bounds.y + 24, bounds.x + bounds.width, bounds.y + 24, Color{55, 55, 60, 255});
    
    Scene* scene = SceneManager::Get().GetActive();
    if (!scene) return;
    
    EntityID e = (EntityID)selectedEntity;
    if (!scene->GetRegistry().valid(e)) {
        DrawText("No entity selected", (int)bounds.x + 10, (int)bounds.y + 35, 12, Color{150, 150, 150, 255});
        return;
    }
    
    float y = bounds.y + 32;
    
    // Tag
    if (scene->HasComponent<Tag>(e)) {
        auto& tag = scene->GetComponent<Tag>(e);
        DrawText("Name:", (int)bounds.x + 10, (int)y + 4, 12, Color{180, 180, 200, 255});
        Rectangle nameBox = {bounds.x + 70, (int)y, bounds.width - 80, 22};
        GuiTextBox(nameBox, tag.name.data(), 64, true);
        y += 30;
    }
    
    RenderTransform(y, selectedEntity);
    RenderSprite(y, selectedEntity);
    RenderCamera(y, selectedEntity);
    RenderScript(y, selectedEntity);
    RenderRigidBody(y, selectedEntity);
    
    // Add component button
    Rectangle addBtn = {bounds.x + 10, y, bounds.width - 20, 24};
    if (GuiButton(addBtn, "+ Add Component")) {
        // For now just add sprite if missing
        if (!scene->HasComponent<SpriteRenderer>(e)) {
            scene->AddComponent<SpriteRenderer>(e);
        } else if (!scene->HasComponent<RigidBody>(e)) {
            scene->AddComponent<RigidBody>(e);
        }
    }
}

void InspectorPanel::RenderTransform(Rectangle& y, uint32_t entity) {
    Scene* scene = SceneManager::Get().GetActive();
    EntityID e = (EntityID)entity;
    if (!scene->HasComponent<Transform>(e)) return;
    
    auto& tf = scene->GetComponent<Transform>(e);
    
    DrawText("Transform", (int)y.x + 10, (int)y + 4, 13, Color{220, 220, 100, 255});
    y.y += 24;
    
    // Position
    DrawText("Pos:", (int)y.x + 10, (int)y + 4, 11, Color{180, 180, 200, 255});
    char buf[64];
    snprintf(buf, sizeof(buf), "X: %.1f  Y: %.1f  Z: %.1f", tf.position.x, tf.position.y, tf.position.z);
    DrawText(buf, (int)y.x + 50, (int)y + 4, 11, Color{220, 220, 220, 255});
    y.y += 20;
    
    DrawText("Rot:", (int)y.x + 10, (int)y + 4, 11, Color{180, 180, 200, 255});
    snprintf(buf, sizeof(buf), "X: %.1f  Y: %.1f  Z: %.1f", tf.rotation.x, tf.rotation.y, tf.rotation.z);
    DrawText(buf, (int)y.x + 50, (int)y + 4, 11, Color{220, 220, 220, 255});
    y.y += 20;
    
    DrawText("Scl:", (int)y.x + 10, (int)y + 4, 11, Color{180, 180, 200, 255});
    snprintf(buf, sizeof(buf), "X: %.1f  Y: %.1f  Z: %.1f", tf.scale.x, tf.scale.y, tf.scale.z);
    DrawText(buf, (int)y.x + 50, (int)y + 4, 11, Color{220, 220, 220, 255});
    y.y += 28;
}

void InspectorPanel::RenderSprite(Rectangle& y, uint32_t entity) {
    Scene* scene = SceneManager::Get().GetActive();
    EntityID e = (EntityID)entity;
    if (!scene->HasComponent<SpriteRenderer>(e)) return;
    
    auto& sr = scene->GetComponent<SpriteRenderer>(e);
    
    DrawText("Sprite Renderer", (int)y.x + 10, (int)y + 4, 13, Color{100, 220, 100, 255});
    y.y += 24;
    
    DrawText("Texture:", (int)y.x + 10, (int)y + 4, 11, Color{180, 180, 200, 255});
    Rectangle texBox = {y.x + 70, (int)y, y.width - 80, 22};
    GuiTextBox(texBox, sr.texturePath.data(), 128, false);
    y.y += 28;
    
    DrawText("Order:", (int)y.x + 10, (int)y + 4, 11, Color{180, 180, 200, 255});
    Rectangle orderBox = {y.x + 70, (int)y, 80, 22};
    GuiSpinner(orderBox, nullptr, &sr.drawOrder, 0, 100, false);
    y.y += 28;
}

void InspectorPanel::RenderCamera(Rectangle& y, uint32_t entity) {
    Scene* scene = SceneManager::Get().GetActive();
    EntityID e = (EntityID)entity;
    if (!scene->HasComponent<CameraComponent>(e)) return;
    
    auto& cc = scene->GetComponent<CameraComponent>(e);
    
    DrawText("Camera", (int)y.x + 10, (int)y + 4, 13, Color{100, 200, 220, 255});
    y.y += 24;
    
    Rectangle cb1 = {y.x + 10, y, 120, 20};
    GuiCheckBox(cb1, "Primary", &cc.isPrimary);
    y.y += 24;
    
    Rectangle cb2 = {y.x + 10, y, 120, 20};
    GuiCheckBox(cb2, "2D Mode", &cc.is2D);
    y.y += 28;
}

void InspectorPanel::RenderScript(Rectangle& y, uint32_t entity) {
    Scene* scene = SceneManager::Get().GetActive();
    EntityID e = (EntityID)entity;
    if (!scene->HasComponent<ScriptComponent>(e)) return;
    
    auto& sc = scene->GetComponent<ScriptComponent>(e);
    
    DrawText("Script", (int)y.x + 10, (int)y + 4, 13, Color{220, 150, 100, 255});
    y.y += 24;
    
    DrawText("Path:", (int)y.x + 10, (int)y + 4, 11, Color{180, 180, 200, 255});
    Rectangle pathBox = {y.x + 50, (int)y, y.width - 60, 22};
    GuiTextBox(pathBox, sc.scriptPath.data(), 128, false);
    y.y += 28;
}

void InspectorPanel::RenderRigidBody(Rectangle& y, uint32_t entity) {
    Scene* scene = SceneManager::Get().GetActive();
    EntityID e = (EntityID)entity;
    if (!scene->HasComponent<RigidBody>(e)) return;
    
    auto& rb = scene->GetComponent<RigidBody>(e);
    
    DrawText("Rigid Body", (int)y.x + 10, (int)y + 4, 13, Color{220, 100, 100, 255});
    y.y += 24;
    
    Rectangle cb1 = {y.x + 10, y, 120, 20};
    GuiCheckBox(cb1, "Use Gravity", &rb.useGravity);
    y.y += 24;
    
    DrawText("Mass:", (int)y.x + 10, (int)y + 4, 11, Color{180, 180, 200, 255});
    Rectangle massBox = {y.x + 60, (int)y, 60, 20};
    float mass = rb.mass;
    GuiSpinner(massBox, nullptr, (int*)&mass, 1, 1000, false);
    rb.mass = mass;
    y.y += 28;
}

} // namespace luminus
