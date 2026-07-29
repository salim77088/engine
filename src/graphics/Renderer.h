// graphics/Renderer.h - Advanced renderer built on raylib's rlgl
#pragma once
#include "raylib.h"
#include "rlgl.h"
#include <memory>

namespace luminus {

class Camera;
class PostProcessor;

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool Init();
    void Shutdown();
    
    void BeginFrame();
    void RenderScene();
    void EndFrame();
    void Present();  // Calls raylib EndDrawing() — must be last call in frame
    
    // 2D primitives
    void DrawSprite2D(Vector3 pos, Vector2 size, Color tint, Texture2D* texture = nullptr, 
                      Rectangle srcRect = {0,0,0,0}, float rotation = 0, Vector2 anchor = {0.5f, 0.5f});
    void DrawText2D(const std::string& text, Vector3 pos, int fontSize, Color color);
    void DrawRect2D(Vector3 pos, Vector2 size, Color color, bool filled = true);
    void DrawCircle2D(Vector3 pos, float radius, Color color, bool filled = true);
    void DrawLine2D(Vector3 a, Vector3 b, Color color, float thickness = 1);
    
    // 3D primitives
    void DrawCube3D(Vector3 pos, Vector3 size, Color color);
    void DrawSphere3D(Vector3 pos, float radius, Color color);
    void DrawModel3D(Vector3 pos, Model& model, Color tint);
    void DrawGrid3D(int slices, float spacing);
    
    // Camera
    void SetPrimaryCamera2D(Camera2D cam);
    void SetPrimaryCamera3D(Camera3D cam);
    
    // Background
    void SetClearColor(Color color) { m_ClearColor = color; }
    
    // Stats
    int GetDrawCalls() const { return m_DrawCalls; }
    int GetVerticesDrawn() const { return m_VerticesDrawn; }
    void ResetStats() { m_DrawCalls = 0; m_VerticesDrawn = 0; }

private:
    Color m_ClearColor {30, 30, 35, 255};
    int m_DrawCalls = 0;
    int m_VerticesDrawn = 0;
    bool m_HasCamera2D = false;
    bool m_HasCamera3D = false;
    Camera2D m_Camera2D {0};
    Camera3D m_Camera3D {0};
    std::unique_ptr<PostProcessor> m_PostProcessor;
};

} // namespace luminus
