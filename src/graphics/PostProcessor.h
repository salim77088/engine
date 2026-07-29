// graphics/PostProcessor.h - Post-processing effects
#pragma once
#include "raylib.h"
#include <string>

namespace luminus {

class PostProcessor {
public:
    PostProcessor();
    ~PostProcessor();
    
    bool Init(int width, int height);
    void Shutdown();
    
    void BeginCapture();
    void EndCapture();
    void Render();
    
    void SetEffect(const std::string& name, bool enabled);
    bool IsEffectEnabled(const std::string& name);
    
    void SetBloomIntensity(float v) { m_BloomIntensity = v; }
    void SetVignetteIntensity(float v) { m_VignetteIntensity = v; }
    void SetChromaticAberration(float v) { m_ChromaticAberration = v; }

private:
    RenderTexture2D m_Target {0};
    bool m_Initialized = false;
    
    bool m_Bloom = false;
    bool m_Vignette = false;
    bool m_ChromaticAberration = false;
    bool m_Grayscale = false;
    bool m_Invert = false;
    
    float m_BloomIntensity = 1.0f;
    float m_VignetteIntensity = 0.5f;
    float m_ChromaticAberration = 0.5f;
};

} // namespace luminus
