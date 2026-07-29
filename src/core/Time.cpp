// core/Time.cpp
#include "Time.h"
#include <chrono>

namespace luminus {

float Time::m_DeltaTime = 0.0f;
float Time::m_FixedDelta = 1.0f / 60.0f;
float Time::m_TotalTime = 0.0f;
float Time::m_TimeScale = 1.0f;
int Time::m_FPS = 0;
float Time::m_FrameTime = 0.0f;
uint64_t Time::m_StartTime = 0;
uint64_t Time::m_LastFrameTime = 0;
float Time::m_FpsAccum = 0.0f;
int Time::m_FpsFrames = 0;

void Time::Init() {
    m_StartTime = Now();
    m_LastFrameTime = m_StartTime;
}

void Time::Update() {
    uint64_t now = Now();
    uint64_t delta = now - m_LastFrameTime;
    m_LastFrameTime = now;
    
    m_DeltaTime = (delta / 1000000.0f) * m_TimeScale;
    m_TotalTime += m_DeltaTime;
    m_FrameTime = delta / 1000000.0f;
    
    m_FpsAccum += m_DeltaTime;
    m_FpsFrames++;
    if (m_FpsAccum >= 0.5f) {
        m_FPS = static_cast<int>(m_FpsFrames / m_FpsAccum);
        m_FpsAccum = 0.0f;
        m_FpsFrames = 0;
    }
}

uint64_t Time::Now() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

double Time::NowSeconds() {
    return Now() / 1000000.0;
}

} // namespace luminus
