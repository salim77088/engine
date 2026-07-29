// main.cpp - Luminus Engine entry point
#include "core/Engine.h"
#include "core/SceneManager.h"
#include "core/Window.h"
#include "core/Time.h"
#include "core/Input.h"
#include "utils/Logger.h"
#include "scripting/LuminusScript.h"
#include "utils/FileIO.h"
#include <csignal>
#include <cstdlib>

using namespace luminus;

void SignalHandler(int sig) {
    LM_FATAL("Engine", "Signal %d received, shutting down", sig);
    Engine::Get().Stop();
}

int main(int argc, char** argv) {
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    
    Logger::Get().EnableConsole(true);
    Logger::Get().SetMinLevel(LogLevel::Trace);
    
    LM_INFO("Main", "=== Luminus Engine v2.0.0 ===");
    LM_INFO("Main", "Built on: Raylib 5.5 + EnTT + raygui 4.0");
    LM_INFO("Main", "Features: LuminusScript, Smart Contracts, ECS, PBR rendering, Editor");
    
    EngineConfig config;
    config.name = "Luminus Engine v2.0";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.fullscreen = false;
    config.vsync = true;
    config.targetFPS = 60;
    config.startEditor = true;
    config.enablePhysics = true;
    config.enableAudio = true;
    config.enableContracts = true;
    
    // Parse command-line args
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--scene" && i + 1 < argc) {
            config.initialScene = argv[++i];
        } else if (arg == "--fullscreen") {
            config.fullscreen = true;
        } else if (arg == "--no-editor") {
            config.startEditor = false;
        } else if (arg == "--width" && i + 1 < argc) {
            config.windowWidth = std::atoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            config.windowHeight = std::atoi(argv[++i]);
        } else if (arg == "--help") {
            printf("Luminus Engine v2.0\n");
            printf("Usage: luminus [options]\n");
            printf("Options:\n");
            printf("  --scene <path>      Load scene on startup\n");
            printf("  --fullscreen        Start in fullscreen\n");
            printf("  --no-editor         Disable editor (game mode)\n");
            printf("  --width <n>         Window width\n");
            printf("  --height <n>        Window height\n");
            return 0;
        }
    }
    
    auto& engine = Engine::Get();
    if (!engine.Init(config)) {
        LM_FATAL("Main", "Engine init failed");
        return 1;
    }
    
    // Load a demo script that demonstrates LuminusScript
    engine.GetScript()->LoadString(R"(
        // Demo LuminusScript - shows language features
        
        var frameCount = 0
        const MAX_FRAMES = 1000000
        
        fun onStart() {
            print("LuminusScript started!")
            print("Engine ready. Try the console panel!")
        }
        
        fun onUpdate(self, dt) {
            frameCount = frameCount + 1
            if (frameCount % 300 == 0) {
                print("Frame: " + frameCount + "  FPS: " + fps())
            }
        }
        
        // Event system
        on "GameStart" -> {
            print("Game started!")
        }
        
        // Smart contract example
        contract "DemoContract" {
            verify true
        }
    )", "demo");
    
    engine.Run();
    engine.Shutdown();
    
    return 0;
}
