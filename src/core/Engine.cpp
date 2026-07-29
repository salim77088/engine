#include "Engine.h"
#include "SceneManager.h"
#include "../graphics/GraphicsAdapter.h"
#include "../audio/AudioSystem.h"
#include "../assets/AssetManager.h"
#include "../editor/Editor.h"
#include "../scripting/LuminusScript.h"

#include "cocos2d.h"

namespace luminus {

Engine& Engine::Instance() {
    static Engine inst;
    return inst;
}

Engine::~Engine() { Shutdown(); }

bool Engine::Init(const EngineConfig& cfg) {
    cfg_ = cfg;

    // Initialize Cocos2d-x application
    cocos2d::Application app;

    // Configure GL context hints via cocos2d::GLViewImpl
    auto director = cocos2d::Director::getInstance();
    auto glview = director->getOpenGLView();
    if (!glview) {
        glview = cocos2d::GLViewImpl::create(cfg_.title);
        director->setOpenGLView(glview);
    }
    glview->setFrameSize(cfg_.width, cfg_.height);
    if (cfg_.fullscreen) {
        glview->setFullscreen();
    }

    director->setDisplayStats(false);
    director->setAnimationInterval(1.0f / cfg_.targetFps);

    // Initialize subsystems
    Assets().Init();
    Audio().Init();

    // Load initial scene
    if (!cfg_.scenePath.empty()) {
        Scenes().LoadFromFile(cfg_.scenePath);
    }

    // Initialize editor if requested
    if (cfg_.editorMode) {
        GetEditor().Init();
    }

    initialized_ = true;
    cocos2d::log("Luminus Engine v1.0.0 initialized");
    cocos2d::log("  Window: %dx%d", cfg_.width, cfg_.height);
    cocos2d::log("  Scene:  %s",    cfg_.scenePath.c_str());
    return true;
}

void Engine::Run() {
    if (!initialized_) return;
    running_ = true;
    // Run cocos2d-x main loop
    cocos2d::Application app;
    app.run();
    running_ = false;
}

void Engine::Shutdown() {
    if (!initialized_) return;
    Audio().Shutdown();
    Assets().Shutdown();
    cocos2d::Director::getInstance()->end();
    initialized_ = false;
}

float Engine::GetFrameTime() const {
    return cocos2d::Director::getInstance()->getDeltaTime();
}

float Engine::GetTime() const {
    return (float)cocos2d::utils::gettime();
}

float Engine::GetFps() const {
    return 1.0f / cocos2d::Director::getInstance()->getDeltaTime();
}

SceneManager&    Engine::Scenes()   { return SceneManager::Instance(); }
GraphicsAdapter& Engine::Graphics() { return GraphicsAdapter::Instance(); }
AudioSystem&     Engine::Audio()    { return AudioSystem::Instance(); }
AssetManager&    Engine::Assets()   { return AssetManager::Instance(); }
Editor&          Engine::GetEditor(){ return Editor::Instance(); }

} // namespace luminus
