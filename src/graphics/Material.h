// graphics/Material.h - PBR-ish material definition
#pragma once
#include "raylib.h"
#include <string>

namespace luminus {

struct Material {
    Color albedo {255, 255, 255, 255};
    float metallic = 0.0f;
    float roughness = 0.5f;
    float emission = 0.0f;
    Color emissiveColor {0, 0, 0, 0};
    std::string albedoTexture;
    std::string normalTexture;
    std::string metallicTexture;
    std::string roughnessTexture;
    std::string emissionTexture;
    bool doubleSided = false;
    bool wireframe = false;
    float alphaCutoff = 0.5f;
    enum class BlendMode { Opaque, Alpha, Additive, Multiply };
    BlendMode blendMode = BlendMode::Opaque;
};

} // namespace luminus
