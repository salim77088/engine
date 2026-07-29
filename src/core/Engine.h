// core/Engine.h - Main engine class
#pragma once
#include <memory>
#include <string>
#include <functional>

namespace luminus {

class Window;
class Renderer;
class Editor;
class AudioSystem;
class AssetManager;
class PhysicsWorld;
class LuminusScript;
class ContractVM;
class Scene;

struct EngineConfig {
    std::string name = "Luminus Engine";
    int windowWidth = 1280;
    int windowHeight = 720;
    bool fullscreen = false;
    bool vsync = true;
    int targetFPS = 60;
    bool startEditor = true;
    bool enablePhysics = true;
    bool enableAudio = true;
    bool enableContracts = true;
    std::string initialScene;
    std::string workingDir;
};

class Engine {
public:
    static Engine& Get();
    
    bool Init(const EngineConfig& config = {});
    void Shutdown();
    void Run();
    void Stop() { m_Running = false; }
    
    // Subsystems accessors
    Window* GetWindow() const { return m_Window.get(); }
    Renderer* GetRenderer() const { return m_Renderer.get(); }
    Editor* GetEditor() const { return m_Editor.get(); }
    AudioSystem* GetAudio() const { return m_Audio.get(); }
    AssetManager* GetAssets() const { return m_Assets.get(); }
    PhysicsWorld* GetPhysics() const { return m_Physics.get(); }
    LuminusScript* GetScript() const { return m_Script.get(); }
    ContractVM* GetContracts() const { return m_Contracts.get(); }
    
    bool IsRunning() const { return m_Running; }
    bool IsEditorMode() const { return m_EditorMode; }
    void SetEditorMode(bool mode) { m_EditorMode = mode; }
    
    const EngineConfig& GetConfig() const { return m_Config; }
    
    // Lifecycle callbacks (for game-side integration)
    std::function<void()> OnStart;
    std::function<void(float)> OnUpdate;
    std::function<void()> OnRender;
    std::function<void()> OnShutdown;

private:
    Engine() = default;
    void Update();
    void Render();
    void ProcessScripts();
    void ProcessPhysics();
    
    EngineConfig m_Config;
    bool m_Initialized = false;
    bool m_Running = false;
    bool m_EditorMode = true;
    
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<Editor> m_Editor;
    std::unique_ptr<AudioSystem> m_Audio;
    std::unique_ptr<AssetManager> m_Assets;
    std::unique_ptr<PhysicsWorld> m_Physics;
    std::unique_ptr<LuminusScript> m_Script;
    std::unique_ptr<ContractVM> m_Contracts;
};

} // namespace luminus
