#pragma once
#include "cocos2d.h"
#include <string>
#include <unordered_map>

namespace luminus {

class AudioSystem {
public:
    static AudioSystem& Instance();
    bool Init();
    void Shutdown();
    bool LoadSound(const std::string& name, const std::string& path);
    bool LoadMusic(const std::string& name, const std::string& path);
    void PlaySound(const std::string& name);
    void PlayMusic(const std::string& name, bool loop = true);
    void StopMusic(const std::string& name);
    void SetMasterVolume(float v);
    void Update();
private:
    AudioSystem() = default;
    std::unordered_map<std::string, unsigned int> sounds_;
    std::unordered_map<std::string, cocos2d::AudioEngine::AudioID> music_;
};

} // namespace luminus
