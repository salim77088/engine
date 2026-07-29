#pragma once
#include <string>
#include <vector>
#include "imgui.h"

namespace luminus {

class Editor {
public:
    static Editor& Instance();
    bool Init();
    void Shutdown();
    void Render();
    void Toggle() { visible_ = !visible_; }
    bool IsVisible() const { return visible_; }

private:
    Editor() = default;
    bool visible_ = true;
    int  selected_ = -1;
    bool showConsole_ = true;
    bool showInspector_ = true;
    bool showOutliner_ = true;
    bool showAssets_ = false;
    char consoleBuffer_[8192] = {0};
    int  consoleLen_ = 0;

    void RenderMainMenuBar();
    void RenderToolbar();
    void RenderOutliner();
    void RenderInspector();
    void RenderConsole();
    void RenderAssetBrowser();
    void LogToConsole(const char* fmt, ...);
};

} // namespace luminus
