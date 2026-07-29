// editor/InspectorPanel.h
#pragma once
#include "raylib.h"
#include "raygui.h"
#include <cstdint>

namespace luminus {

class InspectorPanel {
public:
    void Init();
    void Update();
    void Render(Rectangle bounds, uint32_t selectedEntity);

private:
    void RenderTransform(Rectangle& y, uint32_t entity);
    void RenderSprite(Rectangle& y, uint32_t entity);
    void RenderCamera(Rectangle& y, uint32_t entity);
    void RenderScript(Rectangle& y, uint32_t entity);
    void RenderRigidBody(Rectangle& y, uint32_t entity);
};

} // namespace luminus
