// audio/AudioSystem.h
#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>

namespace luminus {

class AudioSystem {
public:
    bool Init();
    void Shutdown();
    void Update();
    
    Sound LoadSound(const std::string& path);
    Music LoadMusic(const std::string& path);
    void PlaySound(const std::string& name);
    void PlayMusic(const std::string& name);
    void StopMusic(const std::string& name);
    void SetMasterVolume(float vol);
    float GetMasterVolume() const;
    
    void PauseAll();
    void ResumeAll();

private:
    std::unordered_map<std::string, Sound> m_Sounds;
    std::unordered_map<std::string, Music> m_Music;
    float m_MasterVolume = 1.0f;
    bool m_Initialized = false;
};

} // namespace luminus
