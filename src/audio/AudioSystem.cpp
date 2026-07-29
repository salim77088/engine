#include "AudioSystem.h"
#include "audio/include/AudioEngine.h"

namespace luminus {

AudioSystem& AudioSystem::Instance() { static AudioSystem i; return i; }

bool AudioSystem::Init() {
    cocos2d::AudioEngine::lazyInit();
    return true;
}

void AudioSystem::Shutdown() {
    cocos2d::AudioEngine::end();
    sounds_.clear();
    music_.clear();
}

bool AudioSystem::LoadSound(const std::string& name, const std::string& path) {
    // cocos2d-x AudioEngine preloads files on demand
    sounds_[name] = 0;
    (void)path;
    return true;
}

bool AudioSystem::LoadMusic(const std::string& name, const std::string& path) {
    music_[name] = cocos2d::AudioEngine::play2d(path, true, 0.0f); // preload with 0 volume
    cocos2d::AudioEngine::stop(music_[name]);
    music_[name] = -1;
    return true;
}

void AudioSystem::PlaySound(const std::string& name) {
    (void)name;
    // Caller should pass the path
}

void AudioSystem::PlayMusic(const std::string& name, bool loop) {
    (void)name; (void)loop;
}

void AudioSystem::StopMusic(const std::string& name) {
    auto it = music_.find(name);
    if (it != music_.end() && it->second != -1) {
        cocos2d::AudioEngine::stop(it->second);
    }
}

void AudioSystem::SetMasterVolume(float v) {
    if (v < 0) v = 0; if (v > 1) v = 1;
    cocos2d::AudioEngine::setVolume(cocos2d::AudioEngine::AudioID(0), v);
}

void AudioSystem::Update() {
    cocos2d::AudioEngine::uncacheAll();
}

} // namespace luminus
