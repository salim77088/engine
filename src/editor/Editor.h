// editor/Editor.h - Visual editor built on raygui
#pragma once
#include "raylib.h"
#include "raygui.h"
#include <memory>
#include <string>

namespace luminus {

class HierarchyPanel;
class InspectorPanel;
class ConsolePanel;
class AssetBrowserPanel;

class Editor {
public:
    Editor();
    ~Editor();
    
    bool Init();
    void Shutdown();
    void Update();
    void Render();
    
    void TogglePlayMode();
    bool IsPlaying() const { return m_Playing; }
    
    void SetSelectedEntity(uint32_t entity) { m_SelectedEntity = entity; }
    uint32_t GetSelectedEntity() const { return m_SelectedEntity; }

private:
    void RenderMainMenu();
    void RenderToolBar();
    void RenderStatusBar();
    void RenderViewport();
    
    bool m_Playing = false;
    bool m_ShowStats = true;
    bool m_Initialized = false;
    
    uint32_t m_SelectedEntity = 0xFFFFFFFF;  // entt::null
    
    std::unique_ptr<HierarchyPanel> m_Hierarchy;
    std::unique_ptr<InspectorPanel> m_Inspector;
    std::unique_ptr<ConsolePanel> m_Console;
    std::unique_ptr<AssetBrowserPanel> m_AssetBrowser;
    
    // Layout
    Rectangle m_MenuRect;
    Rectangle m_ToolBarRect;
    Rectangle m_StatusBarRect;
    Rectangle m_ViewportRect;
    Rectangle m_HierarchyRect;
    Rectangle m_InspectorRect;
    Rectangle m_ConsoleRect;
    Rectangle m_AssetBrowserRect;
    
    void UpdateLayout();
};

} // namespace luminus
