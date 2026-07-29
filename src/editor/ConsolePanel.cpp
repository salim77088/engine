// editor/ConsolePanel.cpp
#include "ConsolePanel.h"
#include "../utils/Logger.h"
#include "../scripting/LuminusScript.h"
#include "../core/Engine.h"
#include <cstring>

namespace luminus {

void ConsolePanel::Init() {
    std::memset(m_InputBuf, 0, sizeof(m_InputBuf));
}

void ConsolePanel::Update() {}

void ConsolePanel::Render(Rectangle bounds) {
    DrawRectangleRec(bounds, Color{25, 25, 30, 255});
    DrawText("CONSOLE", (int)bounds.x + 10, (int)bounds.y + 6, 14, Color{200, 200, 210, 255});
    DrawLine(bounds.x, bounds.y + 24, bounds.x + bounds.width, bounds.y + 24, Color{55, 55, 60, 255});
    
    // Show recent log entries
    const auto& entries = Logger::Get().GetEntries();
    float y = bounds.y + 30;
    int startIdx = entries.size() > 30 ? (int)entries.size() - 30 : 0;
    
    for (int i = startIdx; i < (int)entries.size() && y < bounds.y + bounds.height - 30; i++) {
        const auto& e = entries[i];
        std::string line = "[" + e.timestamp + "] " + e.message;
        Color col = Color{200, 200, 200, 255};
        switch (e.level) {
            case LogLevel::Trace: col = Color{120, 120, 120, 255}; break;
            case LogLevel::Info: col = Color{200, 220, 200, 255}; break;
            case LogLevel::Warn: col = Color{220, 200, 100, 255}; break;
            case LogLevel::Error: col = Color{220, 100, 100, 255}; break;
            case LogLevel::Fatal: col = Color{220, 100, 220, 255}; break;
        }
        DrawText(line.c_str(), (int)bounds.x + 10, (int)y, 10, col);
        y += 14;
    }
    
    // Input box at bottom
    Rectangle input = {bounds.x + 4, bounds.y + bounds.height - 24, bounds.width - 8, 20};
    GuiTextBox(input, m_InputBuf, 1023, m_Focused);
    
    if (IsKeyPressed(KEY_ENTER) && m_Focused && strlen(m_InputBuf) > 0) {
        LM_INFO("Console", "> %s", m_InputBuf);
        
        // Try to execute as LuminusScript
        Engine::Get().GetScript()->LoadString(m_InputBuf, "<console>");
        
        std::memset(m_InputBuf, 0, sizeof(m_InputBuf));
    }
}

} // namespace luminus
