// graphics/Camera.h
#pragma once
#include "raylib.h"
#include "../core/SceneManager.h"

namespace luminus {

class CameraUtility {
public:
    static void Follow2D(EntityID target, float lerp = 0.1f, Vector2 offset = {0,0});
    static void Follow3D(EntityID target, Vector3 offset);
    static void ZoomTo(float zoom);
    static void Shake(float intensity, float duration);
    static void Update(float dt);
};

} // namespace luminus
