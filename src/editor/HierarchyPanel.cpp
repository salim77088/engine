// editor/HierarchyPanel.cpp
#include "HierarchyPanel.h"
#include "../core/SceneManager.h"
#include "../utils/Logger.h"

namespace luminus {

void HierarchyPanel::Init() {}
void HierarchyPanel::Update() {}

void HierarchyPanel::Render(Rectangle bounds, uint32_t& selectedEntity) {
    DrawRectangleRec(bounds, Color{35, 35, 40, 255});
    DrawText("HIERARCHY", (int)bounds.x + 10, (int)bounds.y + 6, 14, Color{200, 200, 210, 255});
    DrawLine(bounds.x, bounds.y + 24, bounds.x + bounds.width, bounds.y + 24, Color{55, 55, 60, 255});
    
    Scene* scene = SceneManager::Get().GetActive();
    if (!scene) {
        DrawText("No active scene", (int)bounds.x + 10, (int)bounds.y + 35, 12, Color{150, 150, 150, 255});
        return;
    }
    
    // Add button
    Rectangle addBtn = {bounds.x + bounds.width - 80, bounds.y + 4, 70, 18};
    if (GuiButton(addBtn, "+ Add")) {
        EntityID e = scene->CreateEntity("Entity");
        selectedEntity = (uint32_t)e;
        LM_INFO("Editor", "Created entity: %u", (uint32_t)e);
    }
    
    // List entities
    float y = bounds.y + 32;
    auto view = scene->GetRegistry().view<Tag>();
    for (auto entity : view) {
        if (y > bounds.y + bounds.height - 20) break;
        
        auto& tag = view.get<Tag>(entity);
        Rectangle row = {bounds.x, y, bounds.width - 4, 22};
        bool isSelected = (selectedEntity == (uint32_t)entity);
        
        if (isSelected) {
            DrawRectangleRec(row, Color{70, 110, 170, 255});
        } else if (CheckCollisionPointRec(GetMousePosition(), row)) {
            DrawRectangleRec(row, Color{50, 50, 60, 255});
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                selectedEntity = (uint32_t)entity;
            }
        }
        
        DrawText(tag.name.c_str(), (int)row.x + 12, (int)row.y + 4, 12, 
                 isSelected ? Color{255, 255, 255, 255} : Color{200, 200, 210, 255});
        
        y += 22;
    }
}

} // namespace luminus
