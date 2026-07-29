// editor/AssetBrowserPanel.cpp
#include "AssetBrowserPanel.h"
#include "../utils/FileIO.h"
#include "../utils/Logger.h"
#include <dirent.h>
#include <cstring>

namespace luminus {

void AssetBrowserPanel::Init() {
    m_CurrentDir = FileIO::GetExeDir();
    Refresh();
}

void AssetBrowserPanel::Update() {}

void AssetBrowserPanel::Refresh() {
    m_Entries.clear();
    DIR* d = opendir(m_CurrentDir.c_str());
    if (!d) return;
    
    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0) continue;
        m_Entries.push_back(entry->d_name);
    }
    closedir(d);
}

void AssetBrowserPanel::Render(Rectangle bounds) {
    DrawRectangleRec(bounds, Color{30, 30, 35, 255});
    DrawText("ASSETS", (int)bounds.x + 10, (int)bounds.y + 6, 14, Color{200, 200, 210, 255});
    DrawLine(bounds.x, bounds.y + 24, bounds.x + bounds.width, bounds.y + 24, Color{55, 55, 60, 255});
    
    // Path display
    DrawText(m_CurrentDir.c_str(), (int)bounds.x + 10, (int)bounds.y + 30, 10, Color{150, 200, 200, 255});
    
    // Refresh button
    Rectangle refreshBtn = {bounds.x + bounds.width - 70, bounds.y + 28, 60, 16};
    if (GuiButton(refreshBtn, "Refresh")) {
        Refresh();
    }
    
    // List entries
    float y = bounds.y + 50;
    for (const auto& entry : m_Entries) {
        if (y > bounds.y + bounds.height - 20) break;
        
        Rectangle row = {bounds.x, y, bounds.width - 4, 18};
        if (CheckCollisionPointRec(GetMousePosition(), row)) {
            DrawRectangleRec(row, Color{50, 50, 60, 255});
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                std::string newPath = FileIO::Join(m_CurrentDir, entry);
                if (FileIO::Exists(newPath)) {
                    // Check if directory by trying to list it
                    DIR* test = opendir(newPath.c_str());
                    if (test) {
                        closedir(test);
                        if (entry == "..") {
                            m_CurrentDir = FileIO::GetDirectory(m_CurrentDir);
                        } else {
                            m_CurrentDir = newPath;
                        }
                        Refresh();
                    }
                }
            }
        }
        
        DrawText(entry.c_str(), (int)row.x + 12, (int)row.y + 2, 11, Color{200, 200, 200, 255});
        y += 18;
    }
}

} // namespace luminus
