// physics/PhysicsWorld.cpp
#include "PhysicsWorld.h"
#include "../utils/Logger.h"
#include <cmath>
#include <algorithm>

namespace luminus {

bool PhysicsWorld::Init() {
    m_Gravity = {0, -9.81f, 0};
    m_Initialized = true;
    LM_INFO("Physics", "PhysicsWorld initialized");
    return true;
}

void PhysicsWorld::Shutdown() {
    m_Contacts.clear();
    m_Initialized = false;
    LM_INFO("Physics", "PhysicsWorld shutdown");
}

bool PhysicsWorld::CheckAABB(const Vector3& posA, const Vector3& sizeA,
                              const Vector3& posB, const Vector3& sizeB,
                              Vector3& normal, float& penetration) {
    Vector3 halfA = {sizeA.x/2, sizeA.y/2, sizeA.z/2};
    Vector3 halfB = {sizeB.x/2, sizeB.y/2, sizeB.z/2};
    
    float dx = posB.x - posA.x;
    float px = (halfA.x + halfB.x) - std::abs(dx);
    if (px <= 0) return false;
    
    float dy = posB.y - posA.y;
    float py = (halfA.y + halfB.y) - std::abs(dy);
    if (py <= 0) return false;
    
    float dz = posB.z - posA.z;
    float pz = (halfA.z + halfB.z) - std::abs(dz);
    if (pz <= 0) return false;
    
    // Find minimum penetration axis
    if (px < py && px < pz) {
        normal = {dx < 0 ? 1.0f : -1.0f, 0, 0};
        penetration = px;
    } else if (py < pz) {
        normal = {0, dy < 0 ? 1.0f : -1.0f, 0};
        penetration = py;
    } else {
        normal = {0, 0, dz < 0 ? 1.0f : -1.0f};
        penetration = pz;
    }
    return true;
}

void PhysicsWorld::Update(float dt) {
    if (!m_Initialized) return;
    (void)dt;
    
    m_Contacts.clear();
    
    Scene* scene = SceneManager::Get().GetActive();
    if (!scene) return;
    
    // Detect collisions between colliders
    auto view = scene->GetRegistry().view<Transform, Collider>();
    auto entities = std::vector<EntityID>(view.begin(), view.end());
    
    for (size_t i = 0; i < entities.size(); i++) {
        for (size_t j = i + 1; j < entities.size(); j++) {
            auto& tfA = view.get<Transform>(entities[i]);
            auto& tfB = view.get<Transform>(entities[j]);
            auto& coA = view.get<Collider>(entities[i]);
            auto& coB = view.get<Collider>(entities[j]);
            
            Vector3 normal, point;
            float penetration;
            
            if (coA.type == Collider::Type::Box && coB.type == Collider::Type::Box) {
                if (CheckAABB(tfA.position, coA.size, tfB.position, coB.size, normal, penetration)) {
                    ContactPoint cp;
                    cp.a = entities[i];
                    cp.b = entities[j];
                    cp.normal = normal;
                    cp.penetration = penetration;
                    cp.point = {
                        (tfA.position.x + tfB.position.x) / 2,
                        (tfA.position.y + tfB.position.y) / 2,
                        (tfA.position.z + tfB.position.z) / 2
                    };
                    m_Contacts.push_back(cp);
                    
                    // Resolution: separate objects (simple)
                    if (!coA.isTrigger && !coB.isTrigger) {
                        auto* rbA = scene->GetRegistry().try_get<RigidBody>(entities[i]);
                        auto* rbB = scene->GetRegistry().try_get<RigidBody>(entities[j]);
                        
                        if (rbA && !rbA->isStatic) {
                            tfA.position.x += normal.x * penetration / 2;
                            tfA.position.y += normal.y * penetration / 2;
                            tfA.position.z += normal.z * penetration / 2;
                            // Bounce
                            if (normal.y != 0) rbA->velocity.y = -rbA->velocity.y * 0.5f;
                            if (normal.x != 0) rbA->velocity.x = -rbA->velocity.x * 0.5f;
                        }
                        if (rbB && !rbB->isStatic) {
                            tfB.position.x -= normal.x * penetration / 2;
                            tfB.position.y -= normal.y * penetration / 2;
                            tfB.position.z -= normal.z * penetration / 2;
                            if (normal.y != 0) rbB->velocity.y = -rbB->velocity.y * 0.5f;
                            if (normal.x != 0) rbB->velocity.x = -rbB->velocity.x * 0.5f;
                        }
                    }
                }
            }
        }
    }
}

bool PhysicsWorld::Raycast2D(Vector2 start, Vector2 dir, float maxDist, EntityID& hitEntity, Vector2& hitPoint) {
    Scene* scene = SceneManager::Get().GetActive();
    if (!scene) return false;
    
    float closest = maxDist;
    bool hit = false;
    
    auto view = scene->GetRegistry().view<Transform, Collider>();
    for (auto entity : view) {
        auto& tf = view.get<Transform>(entity);
        auto& co = view.get<Collider>(entity);
        
        // Simple ray-AABB test in 2D
        Vector2 min = {tf.position.x - co.size.x/2, tf.position.y - co.size.y/2};
        Vector2 max = {tf.position.x + co.size.x/2, tf.position.y + co.size.y/2};
        
        float tmin = 0;
        float tmax = maxDist;
        
        for (int i = 0; i < 2; i++) {
            float d = (i == 0) ? dir.x : dir.y;
            float mn = (i == 0) ? min.x : min.y;
            float mx = (i == 0) ? max.x : max.y;
            float s = (i == 0) ? start.x : start.y;
            
            if (std::abs(d) < 1e-6f) {
                if (s < mn || s > mx) { tmin = 1; break; }
            } else {
                float t1 = (mn - s) / d;
                float t2 = (mx - s) / d;
                if (t1 > t2) std::swap(t1, t2);
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                if (tmin > tmax) break;
            }
        }
        
        if (tmin <= tmax && tmin < closest && tmin >= 0) {
            closest = tmin;
            hitEntity = entity;
            hitPoint = {start.x + dir.x * tmin, start.y + dir.y * tmin};
            hit = true;
        }
    }
    
    return hit;
}

bool PhysicsWorld::Raycast3D(Vector3 start, Vector3 dir, float maxDist, EntityID& hitEntity, Vector3& hitPoint) {
    (void)start; (void)dir; (void)maxDist; (void)hitEntity; (void)hitPoint;
    return false;  // TODO
}

std::vector<EntityID> PhysicsWorld::QueryAABB(Vector3 min, Vector3 max) {
    std::vector<EntityID> result;
    Scene* scene = SceneManager::Get().GetActive();
    if (!scene) return result;
    
    auto view = scene->GetRegistry().view<Transform, Collider>();
    for (auto entity : view) {
        auto& tf = view.get<Transform>(entity);
        auto& co = view.get<Collider>(entity);
        
        Vector3 emin = {tf.position.x - co.size.x/2, tf.position.y - co.size.y/2, tf.position.z - co.size.z/2};
        Vector3 emax = {tf.position.x + co.size.x/2, tf.position.y + co.size.y/2, tf.position.z + co.size.z/2};
        
        if (emin.x <= max.x && emax.x >= min.x &&
            emin.y <= max.y && emax.y >= min.y &&
            emin.z <= max.z && emax.z >= min.z) {
            result.push_back(entity);
        }
    }
    return result;
}

} // namespace luminus
