// graphics/Mesh.h - Mesh data
#pragma once
#include "raylib.h"
#include <string>
#include <vector>

namespace luminus {

struct Mesh {
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<unsigned int> indices;
    
    // Built-in primitives
    static Mesh Quad(float w, float h);
    static Mesh Cube(float size);
    static Mesh Sphere(float radius, int segments);
    static Mesh Plane(float w, float d);
    
    // Convert to raylib Mesh
    ::Mesh ToRaylibMesh() const;
    
    void Upload();
    void Unload();
    
    ::Mesh rMesh {0};
    bool uploaded = false;
};

} // namespace luminus
