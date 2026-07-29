// graphics/PostProcessor.cpp
#include "PostProcessor.h"
#include "../utils/Logger.h"

namespace luminus {

PostProcessor::PostProcessor() {}
PostProcessor::~PostProcessor() { Shutdown(); }

bool PostProcessor::Init(int width, int height) {
    m_Target = LoadRenderTexture(width, height);
    SetTextureFilter(m_Target.texture, TEXTURE_FILTER_BILINEAR);
    m_Initialized = true;
    LM_INFO("PostProcessor", "Initialized (%dx%d)", width, height);
    return true;
}

void PostProcessor::Shutdown() {
    if (m_Initialized) {
        UnloadRenderTexture(m_Target);
        m_Initialized = false;
    }
}

void PostProcessor::BeginCapture() {
    if (!m_Initialized) return;
    BeginTextureMode(m_Target);
    ClearBackground(BLANK);
}

void PostProcessor::EndCapture() {
    if (!m_Initialized) return;
    EndTextureMode();
}

void PostProcessor::Render() {
    if (!m_Initialized) return;
    
    // Simple post-processing via DrawTexture with optional effects
    Rectangle src = {0, 0, (float)m_Target.texture.width, (float)-m_Target.texture.height};
    Rectangle dst = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
    
    Color tint = WHITE;
    if (m_Grayscale) tint = Color{128, 128, 128, 255};
    if (m_Invert) tint = Color{255, 255, 255, 128};
    
    DrawTexturePro(m_Target.texture, src, dst, {0,0}, 0.0f, tint);
    
    // Vignette overlay
    if (m_Vignette && m_VignetteIntensity > 0) {
        Color c = Color{0, 0, 0, (unsigned char)(m_VignetteIntensity * 100)};
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), c);
    }
}

void PostProcessor::SetEffect(const std::string& name, bool enabled) {
    if (name == "bloom") m_Bloom = enabled;
    else if (name == "vignette") m_Vignette = enabled;
    else if (name == "chromatic") m_ChromaticAberration = enabled;
    else if (name == "grayscale") m_Grayscale = enabled;
    else if (name == "invert") m_Invert = enabled;
}

bool PostProcessor::IsEffectEnabled(const std::string& name) {
    if (name == "bloom") return m_Bloom;
    if (name == "vignette") return m_Vignette;
    if (name == "chromatic") return m_ChromaticAberration;
    if (name == "grayscale") return m_Grayscale;
    if (name == "invert") return m_Invert;
    return false;
}

} // namespace luminus
