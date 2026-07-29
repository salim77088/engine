#include "GraphicsAdapter.h"
#include <algorithm>
#include <cctype>

namespace luminus {

GraphicsAdapter& GraphicsAdapter::Instance() {
    static GraphicsAdapter inst;
    return inst;
}

bool GraphicsAdapter::Init() {
    initialized_ = true;
    return true;
}

void GraphicsAdapter::Shutdown() { initialized_ = false; }

void GraphicsAdapter::BeginFrame() {}
void GraphicsAdapter::EndFrame()   {}

void GraphicsAdapter::ClearColor(const std::string& colorName) {
    cocos2d::Director::getInstance()->setClearColor(ColorFromName(colorName));
}

cocos2d::Color3B GraphicsAdapter::ColorFromName(const std::string& name) {
    std::string n = name;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);
    if (n == "WHITE")       return cocos2d::Color3B::WHITE;
    if (n == "BLACK")       return cocos2d::Color3B::BLACK;
    if (n == "RED")         return cocos2d::Color3B::RED;
    if (n == "GREEN")       return cocos2d::Color3B::GREEN;
    if (n == "BLUE")        return cocos2d::Color3B::BLUE;
    if (n == "YELLOW")      return cocos2d::Color3B::YELLOW;
    if (n == "ORANGE")      return cocos2d::Color3B::ORANGE;
    if (n == "GRAY" || n == "GREY") return cocos2d::Color3B::GRAY;
    if (n == "MAGENTA")     return cocos2d::Color3B::MAGENTA;
    if (n == "DARKGRAY")    return cocos2d::Color3B(40, 40, 40);
    if (n == "LIGHTGRAY")   return cocos2d::Color3B(200, 200, 200);
    if (n == "SKYBLUE")     return cocos2d::Color3B(135, 206, 235);
    if (n == "PURPLE")      return cocos2d::Color3B(128, 0, 128);
    if (n == "PINK")        return cocos2d::Color3B(255, 192, 203);
    if (n == "GOLD")        return cocos2d::Color3B(255, 215, 0);
    if (n == "BROWN")       return cocos2d::Color3B(139, 69, 19);
    return cocos2d::Color3B::WHITE;
}

void GraphicsAdapter::DrawSprite(const std::string& texture, float x, float y, float w, float h, float rot, float scale, float opacity) {
    auto sprite = cocos2d::Sprite::create(texture);
    if (!sprite) return;
    sprite->setPosition(cocos2d::Vec2(x, y));
    if (w > 0 && h > 0) sprite->setContentSize(cocos2d::Size(w, h));
    sprite->setRotation(rot);
    sprite->setScale(scale);
    sprite->setOpacity((GLubyte)(opacity * 255));
    cocos2d::Director::getInstance()->getRunningScene()->addChild(sprite);
}

void GraphicsAdapter::DrawText2D(const std::string& text, float x, float y, int fontSize, const std::string& color, const std::string& fontPath) {
    cocos2d::Label* label;
    if (!fontPath.empty()) {
        label = cocos2d::Label::createWithTTF(text, fontPath, fontSize);
    } else {
        label = cocos2d::Label::createWithSystemFont(text, "Arial", fontSize);
    }
    if (!label) return;
    label->setPosition(cocos2d::Vec2(x, y));
    label->setColor(ColorFromName(color));
    cocos2d::Director::getInstance()->getRunningScene()->addChild(label);
}

void GraphicsAdapter::DrawRect(float x, float y, float w, float h, const std::string& color, bool filled) {
    auto dn = cocos2d::DrawNode::create();
    cocos2d::Vec2 origin(x, y);
    cocos2d::Vec2 dest(x + w, y + h);
    if (filled) {
        dn->drawSolidRect(origin, dest, cocos2d::Color4F(ColorFromName(color)));
    } else {
        dn->drawRect(origin, dest, cocos2d::Color4F(ColorFromName(color)));
    }
    cocos2d::Director::getInstance()->getRunningScene()->addChild(dn);
}

void GraphicsAdapter::DrawCircle(float x, float y, float radius, const std::string& color) {
    auto dn = cocos2d::DrawNode::create();
    dn->drawSolidCircle(cocos2d::Vec2(x, y), radius, 0, 16, cocos2d::Color4F(ColorFromName(color)));
    cocos2d::Director::getInstance()->getRunningScene()->addChild(dn);
}

void GraphicsAdapter::DrawLine(float x1, float y1, float x2, float y2, const std::string& color) {
    auto dn = cocos2d::DrawNode::create();
    dn->drawLine(cocos2d::Vec2(x1, y1), cocos2d::Vec2(x2, y2), cocos2d::Color4F(ColorFromName(color)));
    cocos2d::Director::getInstance()->getRunningScene()->addChild(dn);
}

} // namespace luminus
