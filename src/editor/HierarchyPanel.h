// editor/HierarchyPanel.h
#pragma once
#include "raylib.h"
#include "raygui.h"
#include <cstdint>

namespace luminus {

class HierarchyPanel {
public:
    void Init();
    void Update();
    void Render(Rectangle bounds, uint32_t& selectedEntity);

private:
    bool m_AddMenuOpen = false;
};

} // namespace luminus
