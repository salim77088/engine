// graphics/Mesh.cpp
#include "Mesh.h"
#include "rlgl.h"
#include "../utils/Logger.h"

namespace luminus {

Mesh Mesh::Quad(float w, float h) {
    Mesh m;
    m.vertices = {{-w/2,-h/2,0}, {w/2,-h/2,0}, {w/2,h/2,0}, {-w/2,h/2,0}};
    m.normals = {{0,0,1},{0,0,1},{0,0,1},{0,0,1}};
    m.uvs = {{0,1},{1,1},{1,0},{0,0}};
    m.indices = {0,1,2, 0,2,3};
    return m;
}

Mesh Mesh::Cube(float size) {
    Mesh m;
    float s = size/2;
    // Front
    m.vertices.insert(m.vertices.end(), {{-s,-s,s},{s,-s,s},{s,s,s},{-s,s,s}});
    m.normals.insert(m.normals.end(), 4, {0,0,1});
    m.uvs.insert(m.uvs.end(), {{0,1},{1,1},{1,0},{0,0}});
    // Back
    m.vertices.insert(m.vertices.end(), {{s,-s,-s},{-s,-s,-s},{-s,s,-s},{s,s,-s}});
    m.normals.insert(m.normals.end(), 4, {0,0,-1});
    m.uvs.insert(m.uvs.end(), {{0,1},{1,1},{1,0},{0,0}});
    // Top
    m.vertices.insert(m.vertices.end(), {{-s,s,s},{s,s,s},{s,s,-s},{-s,s,-s}});
    m.normals.insert(m.normals.end(), 4, {0,1,0});
    m.uvs.insert(m.uvs.end(), {{0,1},{1,1},{1,0},{0,0}});
    // Bottom
    m.vertices.insert(m.vertices.end(), {{-s,-s,-s},{s,-s,-s},{s,-s,s},{-s,-s,s}});
    m.normals.insert(m.normals.end(), 4, {0,-1,0});
    m.uvs.insert(m.uvs.end(), {{0,1},{1,1},{1,0},{0,0}});
    // Right
    m.vertices.insert(m.vertices.end(), {{s,-s,s},{s,-s,-s},{s,s,-s},{s,s,s}});
    m.normals.insert(m.normals.end(), 4, {1,0,0});
    m.uvs.insert(m.uvs.end(), {{0,1},{1,1},{1,0},{0,0}});
    // Left
    m.vertices.insert(m.vertices.end(), {{-s,-s,-s},{-s,-s,s},{-s,s,s},{-s,s,-s}});
    m.normals.insert(m.normals.end(), 4, {-1,0,0});
    m.uvs.insert(m.uvs.end(), {{0,1},{1,1},{1,0},{0,0}});
    
    for (int i = 0; i < 6; i++) {
        int off = i*4;
        m.indices.insert(m.indices.end(), {off,off+1,off+2, off,off+2,off+3});
    }
    return m;
}

Mesh Mesh::Sphere(float radius, int segments) {
    Mesh m;
    int rings = segments / 2;
    for (int r = 0; r <= rings; r++) {
        float v = (float)r / rings;
        float phi = v * PI;
        for (int s = 0; s <= segments; s++) {
            float u = (float)s / segments;
            float theta = u * 2 * PI;
            float x = cosf(theta) * sinf(phi) * radius;
            float y = cosf(phi) * radius;
            float z = sinf(theta) * sinf(phi) * radius;
            m.vertices.push_back({x,y,z});
            m.normals.push_back({x/radius, y/radius, z/radius});
            m.uvs.push_back({u, v});
        }
    }
    for (int r = 0; r < rings; r++) {
        for (int s = 0; s < segments; s++) {
            int i1 = r * (segments + 1) + s;
            int i2 = i1 + segments + 1;
            m.indices.insert(m.indices.end(), {i1, i2, i1+1, i2, i2+1, i1+1});
        }
    }
    return m;
}

Mesh Mesh::Plane(float w, float d) {
    Mesh m;
    m.vertices = {{-w/2,0,-d/2}, {w/2,0,-d/2}, {w/2,0,d/2}, {-w/2,0,d/2}};
    m.normals = {{0,1,0},{0,1,0},{0,1,0},{0,1,0}};
    m.uvs = {{0,0},{1,0},{1,1},{0,1}};
    m.indices = {0,1,2, 0,2,3};
    return m;
}

::Mesh Mesh::ToRaylibMesh() const {
    ::Mesh rm = {0};
    rm.vertexCount = (int)vertices.size();
    rm.triangleCount = (int)indices.size() / 3;
    
    rm.vertices = (float*)RL_MALLOC(vertices.size() * 3 * sizeof(float));
    for (size_t i = 0; i < vertices.size(); i++) {
        rm.vertices[i*3] = vertices[i].x;
        rm.vertices[i*3+1] = vertices[i].y;
        rm.vertices[i*3+2] = vertices[i].z;
    }
    
    if (!normals.empty()) {
        rm.normals = (float*)RL_MALLOC(normals.size() * 3 * sizeof(float));
        for (size_t i = 0; i < normals.size(); i++) {
            rm.normals[i*3] = normals[i].x;
            rm.normals[i*3+1] = normals[i].y;
            rm.normals[i*3+2] = normals[i].z;
        }
    }
    
    if (!uvs.empty()) {
        rm.texcoords = (float*)RL_MALLOC(uvs.size() * 2 * sizeof(float));
        for (size_t i = 0; i < uvs.size(); i++) {
            rm.texcoords[i*2] = uvs[i].x;
            rm.texcoords[i*2+1] = uvs[i].y;
        }
    }
    
    if (!indices.empty()) {
        rm.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));
        for (size_t i = 0; i < indices.size(); i++) {
            rm.indices[i] = (unsigned short)indices[i];
        }
    }
    
    return rm;
}

void Mesh::Upload() {
    if (uploaded) return;
    rMesh = ToRaylibMesh();
    UploadMesh(&rMesh, false);
    uploaded = true;
}

void Mesh::Unload() {
    if (uploaded) {
        UnloadMesh(rMesh);
        uploaded = false;
    }
}

} // namespace luminus
