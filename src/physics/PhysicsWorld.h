// physics/PhysicsWorld.h
#pragma once
#include "raylib.h"
#include "../core/SceneManager.h"
#include <vector>

namespace luminus {

struct ContactPoint {
    EntityID a;
    EntityID b;
    Vector3 point;
    Vector3 normal;
    float penetration;
};

class PhysicsWorld {
public:
    bool Init();
    void Shutdown();
    void Update(float dt);
    
    void SetGravity(Vector3 g) { m_Gravity = g; }
    Vector3 GetGravity() const { return m_Gravity; }
    
    const std::vector<ContactPoint>& GetContacts() const { return m_Contacts; }
    
    // Ray casting
    bool Raycast2D(Vector2 start, Vector2 dir, float maxDist, EntityID& hitEntity, Vector2& hitPoint);
    bool Raycast3D(Vector3 start, Vector3 dir, float maxDist, EntityID& hitEntity, Vector3& hitPoint);
    
    // AABB queries
    std::vector<EntityID> QueryAABB(Vector3 min, Vector3 max);

private:
    Vector3 m_Gravity {0, -9.81f, 0};
    std::vector<ContactPoint> m_Contacts;
    bool m_Initialized = false;
    
    bool CheckAABB(const Vector3& posA, const Vector3& sizeA, 
                   const Vector3& posB, const Vector3& sizeB,
                   Vector3& normal, float& penetration);
};

} // namespace luminus
