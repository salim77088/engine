// core/Input.cpp
#include "Input.h"

namespace luminus {

void Input::Update() {
    // raylib handles input polling internally
}

bool Input::IsKeyDown(int key) { return ::IsKeyDown(key); }
bool Input::IsKeyPressed(int key) { return ::IsKeyPressed(key); }
bool Input::IsKeyReleased(int key) { return ::IsKeyReleased(key); }
int Input::GetKeyPressed() { return ::GetKeyPressed(); }
int Input::GetCharPressed() { return ::GetCharPressed(); }

bool Input::IsMouseButtonDown(int button) { return ::IsMouseButtonDown(button); }
bool Input::IsMouseButtonPressed(int button) { return ::IsMouseButtonPressed(button); }
bool Input::IsMouseButtonReleased(int button) { return ::IsMouseButtonReleased(button); }
int Input::GetMouseX() { return ::GetMouseX(); }
int Input::GetMouseY() { return ::GetMouseY(); }
int Input::GetMouseWheelMove() { return (int)::GetMouseWheelMove(); }
float Input::GetMouseWheelMoveV() { return ::GetMouseWheelMove(); }

void Input::SetMouseCursor(int cursor) { ::SetMouseCursor(cursor); }
void Input::ShowCursor() { ::ShowCursor(); }
void Input::HideCursor() { ::HideCursor(); }
bool Input::IsCursorHidden() { return ::IsCursorHidden(); }
void Input::DisableCursor() { ::DisableCursor(); }
void Input::EnableCursor() { ::EnableCursor(); }

bool Input::IsGamepadAvailable(int gamepad) { return ::IsGamepadAvailable(gamepad); }
bool Input::IsGamepadButtonDown(int gamepad, int button) { return ::IsGamepadButtonDown(gamepad, button); }
bool Input::IsGamepadButtonPressed(int gamepad, int button) { return ::IsGamepadButtonPressed(gamepad, button); }
float Input::GetGamepadAxisMovement(int gamepad, int axis) { return ::GetGamepadAxisMovement(gamepad, axis); }
std::string Input::GetGamepadName(int gamepad) { return std::string(::GetGamepadName(gamepad)); }

int Input::GetTouchX() { return ::GetTouchX(); }
int Input::GetTouchY() { return ::GetTouchY(); }
int Input::GetTouchPointCount() { return ::GetTouchPointCount(); }

} // namespace luminus
