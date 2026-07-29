// graphics/Renderer.cpp
#include "Renderer.h"
#include "../core/SceneManager.h"
#include "../core/Time.h"
#include "../utils/Logger.h"
#include "../assets/AssetManager.h"
#include <algorithm>
#include <string>

namespace luminus {

Renderer::Renderer() {}
Renderer::~Renderer() {}

bool Renderer::Init() {
    m_ClearColor = Color{30, 30, 35, 255};
    LM_INFO("Renderer", "Renderer initialized (raylib + rlgl backend)");
    LM_INFO("Renderer", "GLSL version: 330");
    return true;
}

void Renderer::Shutdown() {
    LM_INFO("Renderer", "Renderer shutdown");
}

void Renderer::SetPrimaryCamera2D(Camera2D cam) {
    m_Camera2D = cam;
    m_HasCamera2D = true;
    m_HasCamera3D = false;
}

void Renderer::SetPrimaryCamera3D(Camera3D cam) {
    m_Camera3D = cam;
    m_HasCamera3D = true;
    m_HasCamera2D = false;
}

void Renderer::BeginFrame() {
    ::BeginDrawing();
    ClearBackground(m_ClearColor);
    ResetStats();
}

void Renderer::RenderScene() {
    Scene* scene = SceneManager::Get().GetActive();
    if (!scene) return;
    
    // Find primary camera
    auto camView = scene->GetRegistry().view<CameraComponent, Transform>();
    CameraComponent* primaryCam = nullptr;
    EntityID camEntity = INVALID_ENTITY;
    Transform* camTf = nullptr;
    
    for (auto entity : camView) {
        auto& cc = camView.get<CameraComponent>(entity);
        if (cc.isPrimary) {
            primaryCam = &cc;
            camEntity = entity;
            camTf = &camView.get<Transform>(entity);
            break;
        }
    }
    
    // If no primary camera, use first one
    if (!primaryCam) {
        for (auto entity : camView) {
            primaryCam = &camView.get<CameraComponent>(entity);
            camEntity = entity;
            camTf = &camView.get<Transform>(entity);
            break;
        }
    }
    
    if (primaryCam && !primaryCam->is2D && camTf) {
        // 3D camera
        Camera3D cam {0};
        cam.position = camTf->position;
        cam.target = {camTf->position.x, camTf->position.y, camTf->position.z + 1};
        cam.up = {0, 1, 0};
        cam.fovy = primaryCam->fov;
        cam.projection = CAMERA_PERSPECTIVE;
        SetPrimaryCamera3D(cam);
        
        BeginMode3D(m_Camera3D);
        
        // Render meshes
        auto meshView = scene->GetRegistry().view<Transform, MeshRenderer>();
        for (auto entity : meshView) {
            auto& tf = meshView.get<Transform>(entity);
            auto& mr = meshView.get<MeshRenderer>(entity);
            if (mr.loaded && mr.model.materials != nullptr) {
                DrawModelEx(mr.model, tf.position, {0,1,0}, 0, tf.scale, mr.tint);
                m_DrawCalls++;
            } else {
                DrawCubeV(tf.position, tf.scale, mr.tint);
                m_DrawCalls++;
            }
        }
        
        // Render lights (debug visualization)
        auto lightView = scene->GetRegistry().view<Transform, Light>();
        for (auto entity : lightView) {
            auto& tf = lightView.get<Transform>(entity);
            auto& l = lightView.get<Light>(entity);
            if (l.type == Light::Type::Point) {
                DrawSphereEx(tf.position, 0.1f, 8, 8, l.color);
            }
        }
        
        EndMode3D();
    } else {
        // 2D camera (default)
        Camera2D cam {0};
        if (primaryCam && camTf) {
            cam.target = {camTf->position.x, camTf->position.y};
            cam.offset = {(float)GetScreenWidth()/2.0f, (float)GetScreenHeight()/2.0f};
            cam.rotation = camTf->rotation.z;
            cam.zoom = 1.0f;
        } else {
            cam.zoom = 1.0f;
        }
        SetPrimaryCamera2D(cam);
        
        BeginMode2D(m_Camera2D);
        
        // Render sprites (sorted by draw order)
        struct SpriteBatch {
            EntityID entity;
            int order;
        };
        std::vector<SpriteBatch> batches;
        auto spriteView = scene->GetRegistry().view<Transform, SpriteRenderer>();
        for (auto entity : spriteView) {
            auto& sr = spriteView.get<SpriteRenderer>(entity);
            batches.push_back({entity, sr.drawOrder});
        }
        std::sort(batches.begin(), batches.end(), [](const SpriteBatch& a, const SpriteBatch& b) {
            return a.order < b.order;
        });
        
        for (auto& b : batches) {
            auto& tf = spriteView.get<Transform>(b.entity);
            auto& sr = spriteView.get<SpriteRenderer>(b.entity);
            
            Vector2 size = {32, 32};  // default
            if (sr.texture.id != 0) {
                size = {(float)sr.texture.width, (float)sr.texture.height};
            }
            
            Rectangle dest = {tf.position.x, tf.position.y, size.x * tf.scale.x, size.y * tf.scale.y};
            Vector2 origin = {dest.width * sr.anchor.x, dest.height * sr.anchor.y};
            
            if (sr.texture.id != 0) {
                Rectangle src = sr.sourceRect;
                if (src.width == 0 || src.height == 0) {
                    src = {0, 0, (float)sr.texture.width, (float)sr.texture.height};
                }
                if (sr.flipX) src.width = -src.width;
                if (sr.flipY) src.height = -src.height;
                ::DrawTexturePro(sr.texture, src, dest, origin, tf.rotation.z, sr.tint);
            } else {
                // No texture - draw colored quad
                ::DrawRectanglePro(
                    {dest.x - origin.x, dest.y - origin.y, dest.width, dest.height},
                    {origin.x, origin.y},
                    tf.rotation.z, sr.tint);
            }
            m_DrawCalls++;
        }
        
        // Render 3D cubes in 2D mode (z=0)
        auto cubeView = scene->GetRegistry().view<Transform, MeshRenderer>();
        for (auto entity : cubeView) {
            auto& tf = cubeView.get<Transform>(entity);
            auto& mr = cubeView.get<MeshRenderer>(entity);
            ::DrawRectangleRec(
                {tf.position.x - tf.scale.x*16, tf.position.y - tf.scale.y*16, 
                 tf.scale.x*32, tf.scale.y*32}, mr.tint);
            m_DrawCalls++;
        }
        
        EndMode2D();
    }
    
    // Render UI text (always in screen space)
    auto textView = scene->GetRegistry().view<Transform, Tag>();
    (void)textView;  // Reserved for future label rendering
}

void Renderer::EndFrame() {
    // Draw stats overlay (inside BeginDrawing/EndDrawing)
    DrawFPS(10, 10);
    ::DrawText(TextFormat("Entities: %d  DrawCalls: %d",
                          SceneManager::Get().GetActive() ? (int)SceneManager::Get().GetActive()->GetEntityCount() : 0,
                          m_DrawCalls), 10, 30, 16, RAYWHITE);
    // NOTE: EndDrawing() is called by Engine after Editor::Render()
}

void Renderer::Present() {
    ::EndDrawing();
}

// Primitive drawing methods - direct raylib wrappers
void Renderer::DrawSprite2D(Vector3 pos, Vector2 size, Color tint, Texture2D* texture, 
                            Rectangle srcRect, float rotation, Vector2 anchor) {
    Rectangle dest = {pos.x, pos.y, size.x, size.y};
    Vector2 origin = {dest.width * anchor.x, dest.height * anchor.y};
    if (texture && texture->id != 0) {
        Rectangle src = srcRect;
        if (src.width == 0 || src.height == 0) {
            src = {0, 0, (float)texture->width, (float)texture->height};
        }
        ::DrawTexturePro(*texture, src, dest, origin, rotation, tint);
    } else {
        ::DrawRectanglePro(
            {dest.x - origin.x, dest.y - origin.y, dest.width, dest.height},
            {origin.x, origin.y}, rotation, tint);
    }
    m_DrawCalls++;
}

void Renderer::DrawText2D(const std::string& text, Vector3 pos, int fontSize, Color color) {
    ::DrawText(text.c_str(), (int)pos.x, (int)pos.y, fontSize, color);
    m_DrawCalls++;
}

void Renderer::DrawRect2D(Vector3 pos, Vector2 size, Color color, bool filled) {
    if (filled) {
        ::DrawRectangleV({pos.x, pos.y}, size, color);
    } else {
        ::DrawRectangleLines((int)pos.x, (int)pos.y, (int)size.x, (int)size.y, color);
    }
    m_DrawCalls++;
}

void Renderer::DrawCircle2D(Vector3 pos, float radius, Color color, bool filled) {
    if (filled) {
        ::DrawCircleV({pos.x, pos.y}, radius, color);
    } else {
        ::DrawCircleLines((int)pos.x, (int)pos.y, (int)radius, color);
    }
    m_DrawCalls++;
}

void Renderer::DrawLine2D(Vector3 a, Vector3 b, Color color, float thickness) {
    ::DrawLineEx({a.x, a.y}, {b.x, b.y}, thickness, color);
    m_DrawCalls++;
}

void Renderer::DrawCube3D(Vector3 pos, Vector3 size, Color color) {
    ::DrawCubeV(pos, size, color);
    ::DrawCubeWiresV(pos, size, Color{color.r/2, color.g/2, color.b/2, 255});
    m_DrawCalls += 2;
}

void Renderer::DrawSphere3D(Vector3 pos, float radius, Color color) {
    ::DrawSphereEx(pos, radius, 16, 16, color);
    m_DrawCalls++;
}

void Renderer::DrawModel3D(Vector3 pos, Model& model, Color tint) {
    ::DrawModel(model, pos, 1.0f, tint);
    m_DrawCalls++;
}

void Renderer::DrawGrid3D(int slices, float spacing) {
    ::DrawGrid(slices, spacing);
    m_DrawCalls++;
}

} // namespace luminus
