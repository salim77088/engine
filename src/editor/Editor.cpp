// editor/Editor.cpp
#include "Editor.h"
#include "HierarchyPanel.h"
#include "InspectorPanel.h"
#include "ConsolePanel.h"
#include "AssetBrowserPanel.h"
#include "../core/Engine.h"
#include "../core/SceneManager.h"
#include "../core/Time.h"
#include "../utils/Logger.h"
#include <cstdio>

namespace luminus {

Editor::Editor() {}
Editor::~Editor() {}

bool Editor::Init() {
    GuiSetStyle(DEFAULT, TEXT_SIZE, 14);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x1e1e22FF);
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, 0x2d2d35FF);
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, 0x3d3d47FF);
    GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, 0x4d4d57FF);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xE0E0E0FF);
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0x424247FF);
    GuiSetStyle(DEFAULT, LINE_COLOR, 0x3d3d47FF);
    
    m_Hierarchy = std::make_unique<HierarchyPanel>();
    m_Inspector = std::make_unique<InspectorPanel>();
    m_Console = std::make_unique<ConsolePanel>();
    m_AssetBrowser = std::make_unique<AssetBrowserPanel>();
    
    m_Hierarchy->Init();
    m_Inspector->Init();
    m_Console->Init();
    m_AssetBrowser->Init();
    
    UpdateLayout();
    
    m_Initialized = true;
    LM_INFO("Editor", "Editor initialized (raygui 4.0)");
    return true;
}

void Editor::Shutdown() {
    if (!m_Initialized) return;
    m_Hierarchy.reset();
    m_Inspector.reset();
    m_Console.reset();
    m_AssetBrowser.reset();
    m_Initialized = false;
    LM_INFO("Editor", "Editor shutdown");
}

void Editor::UpdateLayout() {
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    
    m_MenuRect = {0, 0, (float)w, 24};
    m_ToolBarRect = {0, 24, (float)w, 32};
    m_StatusBarRect = {0, (float)h - 20, (float)w, 20};
    
    float panelWidth = 250;
    m_HierarchyRect = {0, 56, panelWidth, (float)h - 56 - 200 - 20};
    m_InspectorRect = {(float)w - panelWidth, 56, panelWidth, (float)h - 56 - 200 - 20};
    m_ConsoleRect = {panelWidth, (float)h - 200 - 20, (float)w - 2*panelWidth, 200};
    m_AssetBrowserRect = {0, (float)h - 200 - 20, panelWidth, 200};
    m_ViewportRect = {panelWidth + 4, 56, (float)w - 2*panelWidth - 8, (float)h - 56 - 200 - 24};
}

void Editor::Update() {
    if (!m_Initialized) return;
    UpdateLayout();
    
    m_Hierarchy->Update();
    m_Inspector->Update();
    m_Console->Update();
    m_AssetBrowser->Update();
}

void Editor::Render() {
    if (!m_Initialized) return;
    
    RenderMainMenu();
    RenderToolBar();
    RenderViewport();
    m_Hierarchy->Render(m_HierarchyRect, m_SelectedEntity);
    m_Inspector->Render(m_InspectorRect, m_SelectedEntity);
    m_Console->Render(m_ConsoleRect);
    m_AssetBrowser->Render(m_AssetBrowserRect);
    RenderStatusBar();
}

void Editor::RenderMainMenu() {
    if (GuiMenuBar(m_MenuRect)) {
        // Handle menu events via state if needed
    }
}

void Editor::RenderToolBar() {
    DrawRectangleRec(m_ToolBarRect, Color{40, 40, 48, 255});
    DrawLine(m_ToolBarRect.x, m_ToolBarRect.y + m_ToolBarRect.height, 
             m_ToolBarRect.x + m_ToolBarRect.width, m_ToolBarRect.y + m_ToolBarRect.height, 
             Color{60, 60, 68, 255});
    
    // Play/Stop button
    Rectangle playBtn = {10, 28, 80, 24};
    const char* playLabel = m_Playing ? "Stop" : "Play";
    if (GuiButton(playBtn, playLabel)) {
        TogglePlayMode();
    }
    
    // Save scene
    Rectangle saveBtn = {100, 28, 80, 24};
    if (GuiButton(saveBtn, "Save")) {
        Scene* s = SceneManager::Get().GetActive();
        if (s) {
            s->Save("examples/untitled.scene.json");
        }
    }
    
    // Load scene
    Rectangle loadBtn = {190, 28, 80, 24};
    if (GuiButton(loadBtn, "Load")) {
        Scene* s = SceneManager::Get().GetActive();
        if (s) {
            s->Load("examples/demo.scene.json");
        }
    }
    
    // Stats toggle
    Rectangle statsBtn = {280, 28, 80, 24};
    if (GuiButton(statsBtn, "Stats")) {
        m_ShowStats = !m_ShowStats;
    }
    
    // Engine name
    DrawText("Luminus Engine v2.0", (int)m_ToolBarRect.width - 180, 34, 16, Color{180, 180, 200, 255});
}

void Editor::RenderViewport() {
    // The viewport is just an outline - actual rendering is the scene
    DrawRectangleLinesEx(m_ViewportRect, 1, Color{80, 80, 90, 255});
}

void Editor::RenderStatusBar() {
    DrawRectangleRec(m_StatusBarRect, Color{30, 30, 35, 255});
    
    char status[256];
    snprintf(status, sizeof(status), "FPS: %d  |  Entities: %d  |  %s  |  Luminus Engine v2.0",
        Time::FPS(),
        SceneManager::Get().GetActive() ? (int)SceneManager::Get().GetActive()->GetEntityCount() : 0,
        m_Playing ? "PLAYING" : "EDITING");
    
    DrawText(status, 10, (int)m_StatusBarRect.y + 4, 12, Color{180, 180, 180, 255});
}

void Editor::TogglePlayMode() {
    m_Playing = !m_Playing;
    Engine::Get().SetEditorMode(!m_Playing);
    LM_INFO("Editor", "Mode: %s", m_Playing ? "PLAY" : "EDIT");
}

} // namespace luminus
