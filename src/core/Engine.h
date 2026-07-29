#pragma once
#include <string>
#include <memory>
#include <functional>

namespace luminus {

struct EngineConfig {
    int          width       = 1280;
    int          height      = 720;
    std::string  title       = "Luminus Engine v1.0.0";
    std::string  scenePath   = "examples/demo.scene.json";
    std::string  scriptPath  = "";
    bool         fullscreen  = false;
    bool         vsync       = true;
    bool         editorMode  = true;   // open editor by default
    int          targetFps   = 60;
    bool         headless    = false;
};

class Engine {
public:
    static Engine& Instance();

    bool Init(const EngineConfig& cfg);
    void Run();
    void Shutdown();

    // Subsystem accessors (defined in their respective modules)
    class SceneManager&   Scenes();
    class GraphicsAdapter& Graphics();
    class AudioSystem&    Audio();
    class AssetManager&   Assets();
    class Editor&         GetEditor();

    const EngineConfig& GetConfig() const { return cfg_; }
    float GetFrameTime() const;
    float GetTime() const;
    float GetFps() const;
    bool  IsRunning() const { return running_; }

    void  Quit() { running_ = false; }

private:
    Engine() = default;
    ~Engine();
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    EngineConfig cfg_;
    bool  initialized_ = false;
    bool  running_     = false;
};

} // namespace luminus
