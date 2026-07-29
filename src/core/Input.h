// core/Input.h - Unified input system
#pragma once
#include "raylib.h"
#include <unordered_map>
#include <string>

namespace luminus {

class Input {
public:
    static void Update();
    
    // Keyboard
    static bool IsKeyDown(int key);
    static bool IsKeyPressed(int key);
    static bool IsKeyReleased(int key);
    static int GetKeyPressed();
    static int GetCharPressed();
    
    // Mouse
    static bool IsMouseButtonDown(int button);
    static bool IsMouseButtonPressed(int button);
    static bool IsMouseButtonReleased(int button);
    static int GetMouseX();
    static int GetMouseY();
    static int GetMouseWheelMove();
    static float GetMouseWheelMoveV();
    
    // Cursor
    static void SetMouseCursor(int cursor);
    static void ShowCursor();
    static void HideCursor();
    static bool IsCursorHidden();
    static void DisableCursor();
    static void EnableCursor();
    
    // Gamepad
    static bool IsGamepadAvailable(int gamepad);
    static bool IsGamepadButtonDown(int gamepad, int button);
    static bool IsGamepadButtonPressed(int gamepad, int button);
    static float GetGamepadAxisMovement(int gamepad, int axis);
    static std::string GetGamepadName(int gamepad);
    
    // Touch
    static int GetTouchX();
    static int GetTouchY();
    static int GetTouchPointCount();

private:
};

} // namespace luminus

// Aliases for raylib constants - makes Luminus feel like its own engine
namespace luminus::Key {
    // Letters
    constexpr int A = KEY_A;
    constexpr int B = KEY_B;
    constexpr int C = KEY_C;
    constexpr int D = KEY_D;
    constexpr int E = KEY_E;
    constexpr int F = KEY_F;
    constexpr int G = KEY_G;
    constexpr int H = KEY_H;
    constexpr int I = KEY_I;
    constexpr int J = KEY_J;
    constexpr int K = KEY_K;
    constexpr int L = KEY_L;
    constexpr int M = KEY_M;
    constexpr int N = KEY_N;
    constexpr int O = KEY_O;
    constexpr int P = KEY_P;
    constexpr int Q = KEY_Q;
    constexpr int R = KEY_R;
    constexpr int S = KEY_S;
    constexpr int T = KEY_T;
    constexpr int U = KEY_U;
    constexpr int V = KEY_V;
    constexpr int W = KEY_W;
    constexpr int X = KEY_X;
    constexpr int Y = KEY_Y;
    constexpr int Z = KEY_Z;
    
    // Numbers
    constexpr int Zero = KEY_ZERO;
    constexpr int One = KEY_ONE;
    constexpr int Two = KEY_TWO;
    constexpr int Three = KEY_THREE;
    constexpr int Four = KEY_FOUR;
    constexpr int Five = KEY_FIVE;
    constexpr int Six = KEY_SIX;
    constexpr int Seven = KEY_SEVEN;
    constexpr int Eight = KEY_EIGHT;
    constexpr int Nine = KEY_NINE;
    
    // Arrows
    constexpr int Up = KEY_UP;
    constexpr int Down = KEY_DOWN;
    constexpr int Left = KEY_LEFT;
    constexpr int Right = KEY_RIGHT;
    
    // Special
    constexpr int Space = KEY_SPACE;
    constexpr int Enter = KEY_ENTER;
    constexpr int Escape = KEY_ESCAPE;
    constexpr int Tab = KEY_TAB;
    constexpr int Backspace = KEY_BACKSPACE;
    constexpr int Delete = KEY_DELETE;
    constexpr int LeftShift = KEY_LEFT_SHIFT;
    constexpr int RightShift = KEY_RIGHT_SHIFT;
    constexpr int LeftControl = KEY_LEFT_CONTROL;
    constexpr int RightControl = KEY_RIGHT_CONTROL;
    constexpr int LeftAlt = KEY_LEFT_ALT;
    constexpr int RightAlt = KEY_RIGHT_ALT;
    
    // Function keys
    constexpr int F1 = KEY_F1;
    constexpr int F2 = KEY_F2;
    constexpr int F3 = KEY_F3;
    constexpr int F4 = KEY_F4;
    constexpr int F5 = KEY_F5;
    constexpr int F6 = KEY_F6;
    constexpr int F7 = KEY_F7;
    constexpr int F8 = KEY_F8;
    constexpr int F9 = KEY_F9;
    constexpr int F10 = KEY_F10;
    constexpr int F11 = KEY_F11;
    constexpr int F12 = KEY_F12;
}

namespace luminus::Mouse {
    constexpr int Left = MOUSE_BUTTON_LEFT;
    constexpr int Right = MOUSE_BUTTON_RIGHT;
    constexpr int Middle = MOUSE_BUTTON_MIDDLE;
    constexpr int Side = MOUSE_BUTTON_SIDE;
    constexpr int Extra = MOUSE_BUTTON_EXTRA;
    constexpr int Forward = MOUSE_BUTTON_FORWARD;
    constexpr int Back = MOUSE_BUTTON_BACK;
}
