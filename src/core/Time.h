// core/Time.h - High-precision time and frame timing
#pragma once
#include <cstdint>

namespace luminus {

class Time {
public:
    static void Init();
    static void Update();  // Call once per frame
    
    static float DeltaTime() { return m_DeltaTime; }
    static float FixedDeltaTime() { return m_FixedDelta; }
    static float TotalTime() { return m_TotalTime; }
    static float TimeScale() { return m_TimeScale; }
    static void SetTimeScale(float scale) { m_TimeScale = scale; }
    
    static int FPS() { return m_FPS; }
    static float FrameTime() { return m_FrameTime; }
    
    static uint64_t Now();  // microseconds since epoch
    static double NowSeconds();

private:
    static float m_DeltaTime;
    static float m_FixedDelta;
    static float m_TotalTime;
    static float m_TimeScale;
    static int m_FPS;
    static float m_FrameTime;
    static uint64_t m_StartTime;
    static uint64_t m_LastFrameTime;
    static float m_FpsAccum;
    static int m_FpsFrames;
};

} // namespace luminus
