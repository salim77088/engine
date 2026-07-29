#include "LuminusScript.h"
#include "Lexer.h"
#include <iostream>
#include "../core/Engine.h"
#include "../core/SceneManager.h"
#include "cocos2d.h"

namespace luminus {

class Interpreter {
public:
    static Interpreter& Instance() {
        static Interpreter i;
        return i;
    }

    void Exec(const ASTNode& n) {
        switch (n.kind) {
            case NodeKind::Assign:        DoAssign(n);          break;
            case NodeKind::IfKeyHeld:     DoIfKey(n, false);    break;
            case NodeKind::IfKeyHit:      DoIfKey(n, true);     break;
            case NodeKind::Say:           std::cout << "[LuminusScript] " << n.strValue << "\n"; break;
            case NodeKind::Spawn:         DoSpawn(n);           break;
            case NodeKind::Destroy:       DoDestroy(n);         break;
            case NodeKind::LoadScene:     Engine::Instance().Scenes().LoadFromFile(n.strValue); break;
            case NodeKind::SetBackground: /* TODO */            break;
            case NodeKind::SetFps:        cocos2d::Director::getInstance()->setAnimationInterval(1.0f / n.numValue); break;
            case NodeKind::Exit:          Engine::Instance().Quit(); break;
        }
    }

private:
    void DoAssign(const ASTNode& n) {
        Entity* e = Engine::Instance().Scenes().FindEntity(n.entity);
        if (!e) return;
        if (n.property == "x")       e->x       = ApplyOp(e->x, n.op, n.numValue);
        else if (n.property == "y")  e->y       = ApplyOp(e->y, n.op, n.numValue);
        else if (n.property == "z")  e->z       = ApplyOp(e->z, n.op, n.numValue);
        else if (n.property == "w" || n.property == "width")  e->width  = ApplyOp(e->width, n.op, n.numValue);
        else if (n.property == "h" || n.property == "height") e->height = ApplyOp(e->height, n.op, n.numValue);
        else if (n.property == "rotation") e->rotation = ApplyOp(e->rotation, n.op, n.numValue);
        else if (n.property == "scale")    e->scale    = ApplyOp(e->scale, n.op, n.numValue);
        else if (n.property == "opacity")  e->opacity  = ApplyOp(e->opacity, n.op, n.numValue);
        else if (n.property == "visible")  e->visible  = (n.numValue != 0);
        else if (n.property == "text")     e->text     = n.strValue;
        else if (n.property == "fontSize") e->fontSize = (int)n.numValue;
        else if (n.property == "color")    e->color    = n.strValue;
        else e->numbers[n.property] = ApplyOp(e->numbers.count(n.property) ? e->numbers[n.property] : 0.0f, n.op, n.numValue);
    }

    float ApplyOp(float cur, const std::string& op, float v) {
        if (op == "=")  return v;
        if (op == "+=") return cur + v;
        if (op == "-=") return cur - v;
        if (op == "*=") return cur * v;
        if (op == "/=") return cur != 0 ? cur / v : 0;
        return v;
    }

    int KeyCode(const std::string& name) {
        if (name == "LEFT")  return (int)cocos2d::EventKeyboard::KeyCode::KEY_LEFT;
        if (name == "RIGHT") return (int)cocos2d::EventKeyboard::KeyCode::KEY_RIGHT;
        if (name == "UP")    return (int)cocos2d::EventKeyboard::KeyCode::KEY_UP;
        if (name == "DOWN")  return (int)cocos2d::EventKeyboard::KeyCode::KEY_DOWN;
        if (name == "SPACE") return (int)cocos2d::EventKeyboard::KeyCode::KEY_SPACE;
        if (name == "ENTER") return (int)cocos2d::EventKeyboard::KeyCode::KEY_ENTER;
        if (name == "ESC")   return (int)cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE;
        if (name.size() == 1) {
            char c = name[0];
            if (c >= 'A' && c <= 'Z') return (int)cocos2d::EventKeyboard::KeyCode::KEY_A + (c - 'A');
            if (c >= '0' && c <= '9') return (int)cocos2d::EventKeyboard::KeyCode::KEY_0 + (c - '0');
        }
        return 0;
    }

    void DoIfKey(const ASTNode& n, bool hit) {
        // Note: cocos2d-x doesn't have a simple "is key down" query; we'd need to listen to events.
        // For now, use a global key state map updated by event listener.
        extern std::unordered_map<int, bool> g_keyHeld;
        extern std::unordered_map<int, bool> g_keyHit;
        int code = KeyCode(n.strValue);
        bool pressed = hit ? (g_keyHit.count(code) && g_keyHit[code])
                           : (g_keyHeld.count(code) && g_keyHeld[code]);
        if (pressed) {
            for (const auto& sub : n.block) Exec(sub);
        }
    }

    void DoSpawn(const ASTNode& n) {
        Entity e;
        e.name = n.strValue;
        Engine::Instance().Scenes().AddEntity(e);
    }

    void DoDestroy(const ASTNode& n) {
        Engine::Instance().Scenes().RemoveEntity(n.strValue);
    }
};

// Global key state (updated by Engine's event listener)
std::unordered_map<int, bool> g_keyHeld;
std::unordered_map<int, bool> g_keyHit;

// Helper to register the interpreter as the script executor
void RegisterInterpreter(LuminusScript& script) {
    script.SetExecutor([](const ASTNode& n){
        Interpreter::Instance().Exec(n);
    });
}

} // namespace luminus
