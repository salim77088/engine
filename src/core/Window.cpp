// core/Window.cpp
#include "Window.h"
#include "utils/Logger.h"
#include "raylib.h"

namespace luminus {

bool Window::Init(const WindowConfig& config) {
    m_Config = config;
    
    // Configure flags BEFORE InitWindow
    if (config.msaa) {
        SetConfigFlags(FLAG_MSAA_4X_HINT);
    }
    if (config.resizable) {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    }
    if (config.vsync) {
        SetConfigFlags(FLAG_VSYNC_HINT);
    }
    if (config.fullscreen) {
        SetConfigFlags(FLAG_FULLSCREEN_MODE);
    }
    
    InitWindow(config.width, config.height, config.title.c_str());
    
    if (!IsWindowReady()) {
        LM_ERROR("Window", "Failed to initialize window");
        return false;
    }
    
    if (config.targetFPS > 0) {
        SetTargetFPS(config.targetFPS);
    }
    
    m_Initialized = true;
    LM_INFO("Window", "Window initialized: %dx%d (%s)", 
            config.width, config.height, config.fullscreen ? "fullscreen" : "windowed");
    return true;
}

void Window::Shutdown() {
    if (m_Initialized) {
        CloseWindow();
        m_Initialized = false;
        LM_INFO("Window", "Window shutdown");
    }
}

bool Window::ShouldClose() {
    return WindowShouldClose();
}

void Window::SwapBuffers() {
    // Raylib handles this internally via BeginDrawing/EndDrawing
}

void Window::PollEvents() {
    // Raylib polls events automatically
}

int Window::GetWidth() const { return GetScreenWidth(); }
int Window::GetHeight() const { return GetScreenHeight(); }
int Window::GetRenderWidth() const { return GetRenderWidth(); }
int Window::GetRenderHeight() const { return GetRenderHeight(); }

void Window::SetTitle(const std::string& title) {
    SetWindowTitle(title.c_str());
    m_Config.title = title;
}

void Window::SetSize(int w, int h) {
    SetWindowSize(w, h);
    m_Config.width = w;
    m_Config.height = h;
}

void Window::SetFullscreen(bool fullscreen) {
    if (fullscreen != IsWindowFullscreen()) {
        ToggleFullscreen();
    }
    m_Config.fullscreen = fullscreen;
}

void Window::SetVSync(bool vsync) {
    // VSync controlled via config flags at init in raylib
    m_Config.vsync = vsync;
}

void Window::SetTargetFPS(int fps) {
    SetTargetFPS(fps);
    m_Config.targetFPS = fps;
}

void Window::SetIcon(const std::string& path) {
    Image img = LoadImage(path.c_str());
    if (img.data) {
        SetWindowIcon(img);
        UnloadImage(img);
    }
}

bool Window::IsFullscreen() const { return IsWindowFullscreen(); }
bool Window::IsVSync() const { return m_Config.vsync; }
int Window::GetFPS() const { return ::GetFPS(); }
float Window::GetFrameTime() const { return ::GetFrameTime(); }

void* Window::GetHandle() const {
    return GetWindowHandle();
}

} // namespace luminus
