// audio/AudioSystem.cpp
#include "AudioSystem.h"
#include "../utils/Logger.h"

namespace luminus {

bool AudioSystem::Init() {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        LM_ERROR("Audio", "Failed to initialize audio device");
        return false;
    }
    m_Initialized = true;
    LM_INFO("Audio", "AudioSystem initialized");
    return true;
}

void AudioSystem::Shutdown() {
    if (!m_Initialized) return;
    for (auto& p : m_Sounds) UnloadSound(p.second);
    for (auto& p : m_Music) UnloadMusicStream(p.second);
    CloseAudioDevice();
    m_Initialized = false;
    LM_INFO("Audio", "AudioSystem shutdown");
}

void AudioSystem::Update() {
    if (!m_Initialized) return;
    for (auto& p : m_Music) {
        UpdateMusicStream(p.second);
    }
}

Sound AudioSystem::LoadSound(const std::string& path) {
    auto it = m_Sounds.find(path);
    if (it != m_Sounds.end()) return it->second;
    Sound s = ::LoadSound(path.c_str());
    m_Sounds[path] = s;
    LM_INFO("Audio", "Loaded sound: %s", path.c_str());
    return s;
}

Music AudioSystem::LoadMusic(const std::string& path) {
    auto it = m_Music.find(path);
    if (it != m_Music.end()) return it->second;
    Music m = ::LoadMusicStream(path.c_str());
    m_Music[path] = m;
    LM_INFO("Audio", "Loaded music: %s", path.c_str());
    return m;
}

void AudioSystem::PlaySound(const std::string& name) {
    auto it = m_Sounds.find(name);
    if (it != m_Sounds.end()) {
        ::PlaySound(it->second);
    }
}

void AudioSystem::PlayMusic(const std::string& name) {
    auto it = m_Music.find(name);
    if (it != m_Music.end()) {
        ::PlayMusicStream(it->second);
    }
}

void AudioSystem::StopMusic(const std::string& name) {
    auto it = m_Music.find(name);
    if (it != m_Music.end()) {
        StopMusicStream(it->second);
    }
}

void AudioSystem::SetMasterVolume(float vol) {
    m_MasterVolume = vol;
    SetMasterVolume(vol);
}

float AudioSystem::GetMasterVolume() const {
    return m_MasterVolume;
}

void AudioSystem::PauseAll() {
    for (auto& p : m_Music) PauseMusicStream(p.second);
}

void AudioSystem::ResumeAll() {
    for (auto& p : m_Music) ResumeMusicStream(p.second);
}

} // namespace luminus
