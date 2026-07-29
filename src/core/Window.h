// core/Window.h - Window management built on Raylib
#pragma once
#include <string>

namespace luminus {

struct WindowConfig {
    std::string title = "Luminus Engine v2.0";
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool resizable = true;
    bool vsync = true;
    int targetFPS = 60;
    bool msaa = true;
    int msaaSamples = 4;
};

class Window {
public:
    bool Init(const WindowConfig& config);
    void Shutdown();
    
    bool ShouldClose();
    void SwapBuffers();
    void PollEvents();
    
    int GetWidth() const;
    int GetHeight() const;
    int GetRenderWidth() const;
    int GetRenderHeight() const;
    
    void SetTitle(const std::string& title);
    void SetSize(int w, int h);
    void SetFullscreen(bool fullscreen);
    void SetVSync(bool vsync);
    void SetTargetFPS(int fps);
    void SetIcon(const std::string& path);
    
    bool IsFullscreen() const;
    bool IsVSync() const;
    int GetFPS() const;
    float GetFrameTime() const;
    
    void* GetHandle() const;  // Native window handle

private:
    WindowConfig m_Config;
    bool m_Initialized = false;
};

} // namespace luminus
