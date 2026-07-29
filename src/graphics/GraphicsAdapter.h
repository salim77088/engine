#pragma once
#include <string>
#include "cocos2d.h"

namespace luminus {

class GraphicsAdapter {
public:
    static GraphicsAdapter& Instance();

    bool Init();
    void Shutdown();
    void BeginFrame();
    void EndFrame();
    void ClearColor(const std::string& colorName);
    void DrawSprite(const std::string& texture, float x, float y, float w = 0, float h = 0, float rot = 0, float scale = 1.0f, float opacity = 1.0f);
    void DrawText2D(const std::string& text, float x, float y, int fontSize, const std::string& color = "WHITE", const std::string& fontPath = "");
    void DrawRect(float x, float y, float w, float h, const std::string& color, bool filled = true);
    void DrawCircle(float x, float y, float radius, const std::string& color);
    void DrawLine(float x1, float y1, float x2, float y2, const std::string& color);

    cocos2d::Color3B ColorFromName(const std::string& name);

private:
    GraphicsAdapter() = default;
    bool initialized_ = false;
};

} // namespace luminus
