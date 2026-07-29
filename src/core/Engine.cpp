// core/Engine.cpp
#include "Engine.h"
#include "Window.h"
#include "Time.h"
#include "Input.h"
#include "SceneManager.h"
#include "utils/Logger.h"
#include "utils/FileIO.h"
#include "../graphics/Renderer.h"
#include "../editor/Editor.h"
#include "../audio/AudioSystem.h"
#include "../assets/AssetManager.h"
#include "../physics/PhysicsWorld.h"
#include "../scripting/LuminusScript.h"
#include "../contracts/ContractVM.h"

namespace luminus {

Engine& Engine::Get() {
    static Engine instance;
    return instance;
}

bool Engine::Init(const EngineConfig& config) {
    m_Config = config;
    
    // Working directory
    if (m_Config.workingDir.empty()) {
        m_Config.workingDir = FileIO::GetExeDir();
    }
    LM_INFO("Engine", "Working directory: %s", m_Config.workingDir.c_str());
    
    // Logger
    Logger::Get().SetLogFile(FileIO::Join(m_Config.workingDir, "luminus.log"));
    Logger::Get().EnableConsole(true);
    
    LM_INFO("Engine", "=== Luminus Engine v2.0.0 ===");
    LM_INFO("Engine", "Initializing...");
    
    // Window
    WindowConfig wc;
    wc.title = m_Config.name;
    wc.width = m_Config.windowWidth;
    wc.height = m_Config.windowHeight;
    wc.fullscreen = m_Config.fullscreen;
    wc.vsync = m_Config.vsync;
    wc.targetFPS = m_Config.targetFPS;
    
    m_Window = std::make_unique<Window>();
    if (!m_Window->Init(wc)) {
        LM_FATAL("Engine", "Window init failed");
        return false;
    }
    
    Time::Init();
    
    // Audio
    if (m_Config.enableAudio) {
        m_Audio = std::make_unique<AudioSystem>();
        m_Audio->Init();
    }
    
    // Assets
    m_Assets = std::make_unique<AssetManager>();
    
    // Renderer
    m_Renderer = std::make_unique<Renderer>();
    m_Renderer->Init();
    
    // Physics
    if (m_Config.enablePhysics) {
        m_Physics = std::make_unique<PhysicsWorld>();
        m_Physics->Init();
    }
    
    // Scripting
    m_Script = std::make_unique<LuminusScript>();
    m_Script->Init();
    
    // Smart contracts
    if (m_Config.enableContracts) {
        m_Contracts = std::make_unique<ContractVM>();
        m_Contracts->Init();
    }
    
    // Editor
    if (m_Config.startEditor) {
        m_Editor = std::make_unique<Editor>();
        m_Editor->Init();
        m_EditorMode = true;
    } else {
        m_EditorMode = false;
    }
    
    // Load initial scene
    if (!m_Config.initialScene.empty()) {
        Scene* scene = SceneManager::Get().NewScene("MainScene");
        if (scene->Load(m_Config.initialScene)) {
            LM_INFO("Engine", "Loaded scene: %s", m_Config.initialScene.c_str());
        }
    } else {
        // Create a default scene with a camera and a sprite
        Scene* scene = SceneManager::Get().NewScene("MainScene");
        auto cam = scene->CreateEntity("MainCamera");
        CameraComponent cc;
        cc.isPrimary = true;
        cc.is2D = true;
        scene->AddComponent(cam, cc);
        
        auto ent = scene->CreateEntity("Player");
        auto& tf = scene->GetComponent<Transform>(ent);
        tf.position = {400, 300, 0};
        SpriteRenderer sr;
        sr.tint = {100, 200, 255, 255};
        sr.drawOrder = 0;
        scene->AddComponent(ent, sr);
    }
    
    m_Initialized = true;
    LM_INFO("Engine", "Initialization complete");
    
    if (OnStart) OnStart();
    
    return true;
}

void Engine::Shutdown() {
    if (!m_Initialized) return;
    
    LM_INFO("Engine", "Shutting down...");
    
    if (OnShutdown) OnShutdown();
    
    // Cleanup in reverse order
    if (m_Editor) m_Editor->Shutdown();
    if (m_Contracts) m_Contracts->Shutdown();
    if (m_Script) m_Script->Shutdown();
    if (m_Physics) m_Physics->Shutdown();
    if (m_Renderer) m_Renderer->Shutdown();
    if (m_Assets) m_Assets.reset();
    if (m_Audio) m_Audio->Shutdown();
    if (m_Window) m_Window->Shutdown();
    
    m_Initialized = false;
    LM_INFO("Engine", "Shutdown complete. Goodbye!");
}

void Engine::Run() {
    if (!m_Initialized) {
        LM_FATAL("Engine", "Cannot run uninitialized engine");
        return;
    }
    
    m_Running = true;
    LM_INFO("Engine", "Main loop started");
    
    while (m_Running && !m_Window->ShouldClose()) {
        Time::Update();
        Update();
        Render();
    }
    
    m_Running = false;
    LM_INFO("Engine", "Main loop ended");
}

void Engine::Update() {
    Input::Update();
    
    // Update audio
    if (m_Audio) m_Audio->Update();
    
    // Update physics
    if (m_Physics) ProcessPhysics();
    
    // Update scripts
    ProcessScripts();
    
    // Update active scene's entities (lifecycle callbacks would go here)
    Scene* scene = SceneManager::Get().GetActive();
    if (scene) {
        // Process RigidBody components
        auto rbView = scene->GetRegistry().view<Transform, RigidBody>();
        for (auto entity : rbView) {
            auto& tf = rbView.get<Transform>(entity);
            auto& rb = rbView.get<RigidBody>(entity);
            
            if (rb.isStatic || rb.isKinematic) continue;
            
            // Gravity
            if (rb.useGravity) {
                rb.acceleration.y += 9.81f * 50.0f * Time::DeltaTime();  // scaled for 2D
            }
            
            // Apply acceleration to velocity
            rb.velocity.x += rb.acceleration.x * Time::DeltaTime();
            rb.velocity.y += rb.acceleration.y * Time::DeltaTime();
            rb.velocity.z += rb.acceleration.z * Time::DeltaTime();
            
            // Drag
            if (rb.drag > 0) {
                rb.velocity.x *= (1.0f - rb.drag * Time::DeltaTime());
                rb.velocity.y *= (1.0f - rb.drag * Time::DeltaTime());
                rb.velocity.z *= (1.0f - rb.drag * Time::DeltaTime());
            }
            
            // Apply velocity to position
            tf.position.x += rb.velocity.x * Time::DeltaTime();
            tf.position.y += rb.velocity.y * Time::DeltaTime();
            tf.position.z += rb.velocity.z * Time::DeltaTime();
            
            // Reset acceleration
            rb.acceleration = {0, 0, 0};
        }
    }
    
    // User callback
    if (OnUpdate) OnUpdate(Time::DeltaTime());
    
    // Editor update
    if (m_Editor && m_EditorMode) m_Editor->Update();
    
    // Contracts tick
    if (m_Contracts) m_Contracts->Tick();
}

void Engine::Render() {
    if (m_Renderer) m_Renderer->BeginFrame();
    
    // User callback (custom rendering)
    if (OnRender) OnRender();
    else if (m_Renderer) m_Renderer->RenderScene();
    
    if (m_Renderer) m_Renderer->EndFrame();
    
    // Editor on top (still inside BeginDrawing/EndDrawing)
    if (m_Editor && m_EditorMode) m_Editor->Render();
    
    // Finally present (calls raylib EndDrawing)
    if (m_Renderer) m_Renderer->Present();
}

void Engine::ProcessScripts() {
    if (!m_Script) return;
    Scene* scene = SceneManager::Get().GetActive();
    if (!scene) return;
    
    auto scriptView = scene->GetRegistry().view<ScriptComponent, Transform>();
    for (auto entity : scriptView) {
        auto& sc = scriptView.get<ScriptComponent>(entity);
        auto& tf = scriptView.get<Transform>(entity);
        
        if (!sc.enabled) continue;
        
        if (!sc.initialized && !sc.scriptPath.empty()) {
            m_Script->LoadFile(sc.scriptPath);
            m_Script->CallFunction("onStart", entity);
            sc.initialized = true;
        }
        
        if (sc.initialized) {
            m_Script->SetEntityContext(entity);
            m_Script->CallFunction("onUpdate", entity, Time::DeltaTime());
        }
    }
}

void Engine::ProcessPhysics() {
    if (!m_Physics) return;
    m_Physics->Update(Time::DeltaTime());
}

} // namespace luminus
