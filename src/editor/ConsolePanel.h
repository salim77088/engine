// editor/ConsolePanel.h
#pragma once
#include "raylib.h"
#include "raygui.h"
#include <string>
#include <vector>

namespace luminus {

class ConsolePanel {
public:
    void Init();
    void Update();
    void Render(Rectangle bounds);

private:
    char m_InputBuf[1024] = {0};
    bool m_Focused = false;
    std::vector<std::string> m_History;
};

} // namespace luminus
