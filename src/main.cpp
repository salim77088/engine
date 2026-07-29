// ============================================================================
//  Luminus Engine v1.0.0 — Built on top of Cocos2d-x v4 (MIT-licensed)
//  Usage:
//    luminus                              -> run with default scene + editor
//    luminus --scene path.json            -> load specific scene
//    luminus --script path.ls             -> load LuminusScript file
//    luminus --no-editor                  -> disable visual editor
//    luminus --width 1920 --height 1080
// ============================================================================

#include "core/Engine.h"
#include "scripting/LuminusScript.h"
#include "scripting/Interpreter.h"
#include <iostream>
#include <string>
#include <vector>

static void PrintHelp() {
    std::cout <<
        "Luminus Engine v1.0.0\n"
        "A powerful lightweight game engine built on Cocos2d-x v4.\n\n"
        "Usage: luminus [options]\n\n"
        "Options:\n"
        "  --scene <path>      Load scene from JSON file\n"
        "  --script <path>     Load LuminusScript (.ls) file\n"
        "  --no-editor         Disable visual editor\n"
        "  --width <n>         Window width  (default 1280)\n"
        "  --height <n>        Window height (default 720)\n"
        "  --fullscreen        Start in fullscreen mode\n"
        "  --fps <n>           Target FPS (default 60)\n"
        "  --help              Show this help\n";
}

int main(int argc, char** argv) {
    luminus::EngineConfig cfg;
    cfg.title      = "Luminus Engine v1.0.0";
    cfg.scenePath  = "examples/demo.scene.json";
    cfg.editorMode = true;

    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "--help" || a == "-h") { PrintHelp(); return 0; }
        else if (a == "--scene"      && i+1 < args.size()) cfg.scenePath = args[++i];
        else if (a == "--script"     && i+1 < args.size()) cfg.scriptPath = args[++i];
        else if (a == "--no-editor")                        cfg.editorMode = false;
        else if (a == "--editor")                           cfg.editorMode = true;
        else if (a == "--width"       && i+1 < args.size()) cfg.width  = std::stoi(args[++i]);
        else if (a == "--height"      && i+1 < args.size()) cfg.height = std::stoi(args[++i]);
        else if (a == "--fullscreen")                       cfg.fullscreen = true;
        else if (a == "--fps"         && i+1 < args.size()) cfg.targetFps = std::stoi(args[++i]);
        else {
            std::cerr << "Unknown argument: " << a << "\n";
            PrintHelp();
            return 1;
        }
    }

    auto& engine = luminus::Engine::Instance();
    if (!engine.Init(cfg)) {
        std::cerr << "Engine initialization failed.\n";
        return 1;
    }

    // Load script if provided
    if (!cfg.scriptPath.empty()) {
        static luminus::LuminusScript script;
        if (script.LoadFromFile(cfg.scriptPath)) {
            luminus::RegisterInterpreter(script);
            script.RunStartHandlers();
            // Note: RunUpdateHandlers should be called every frame; this needs to be wired into the engine's main loop
            std::cout << "[LuminusScript] Loaded " << cfg.scriptPath << "\n";
        } else {
            std::cerr << "[LuminusScript] Error: " << script.Error() << "\n";
        }
    }

    engine.Run();
    return 0;
}
