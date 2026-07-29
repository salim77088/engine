// editor/AssetBrowserPanel.h
#pragma once
#include "raylib.h"
#include "raygui.h"
#include <string>
#include <vector>

namespace luminus {

class AssetBrowserPanel {
public:
    void Init();
    void Update();
    void Render(Rectangle bounds);

private:
    std::string m_CurrentDir;
    std::vector<std::string> m_Entries;
    void Refresh();
};

} // namespace luminus
