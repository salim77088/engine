// graphics/Camera.cpp
#include "Camera.h"
#include "../core/Time.h"
#include "../utils/Logger.h"
#include <cmath>

namespace luminus {

namespace {
    Vector3 g_ShakeOffset {0,0,0};
    float g_ShakeIntensity = 0;
    float g_ShakeDuration = 0;
}

void CameraUtility::Follow2D(EntityID target, float lerp, Vector2 offset) {
    Scene* s = SceneManager::Get().GetActive();
    if (!s || !s->HasComponent<Transform>(target)) return;
    auto& tf = s->GetComponent<Transform>(target);
    auto view = s->GetRegistry().view<CameraComponent, Transform>();
    for (auto e : view) {
        auto& cc = view.get<CameraComponent>(e);
        if (cc.isPrimary && cc.is2D) {
            auto& camTf = view.get<Transform>(e);
            camTf.position.x += (tf.position.x + offset.x - camTf.position.x) * lerp;
            camTf.position.y += (tf.position.y + offset.y - camTf.position.y) * lerp;
        }
    }
}

void CameraUtility::Follow3D(EntityID target, Vector3 offset) {
    Scene* s = SceneManager::Get().GetActive();
    if (!s || !s->HasComponent<Transform>(target)) return;
    auto& tf = s->GetComponent<Transform>(target);
    auto view = s->GetRegistry().view<CameraComponent, Transform>();
    for (auto e : view) {
        auto& cc = view.get<CameraComponent>(e);
        if (cc.isPrimary && !cc.is2D) {
            auto& camTf = view.get<Transform>(e);
            camTf.position = {tf.position.x + offset.x, tf.position.y + offset.y, tf.position.z + offset.z};
        }
    }
}

void CameraUtility::ZoomTo(float zoom) {
    // Would update camera zoom - kept for completeness
    (void)zoom;
}

void CameraUtility::Shake(float intensity, float duration) {
    g_ShakeIntensity = intensity;
    g_ShakeDuration = duration;
}

void CameraUtility::Update(float dt) {
    if (g_ShakeDuration > 0) {
        g_ShakeDuration -= dt;
        g_ShakeOffset.x = (rand() / (float)RAND_MAX - 0.5f) * g_ShakeIntensity * 2;
        g_ShakeOffset.y = (rand() / (float)RAND_MAX - 0.5f) * g_ShakeIntensity * 2;
    } else {
        g_ShakeOffset = {0,0,0};
    }
}

} // namespace luminus
