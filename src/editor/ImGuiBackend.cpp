#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "cocos2d.h"

namespace luminus {

bool InitImGuiBackend(cocos2d::GLView* view) {
    // cocos2d-x v4 uses GLFW on desktop
    auto* glfwView = dynamic_cast<cocos2d::GLViewImpl*>(view);
    if (!glfwView) return false;
    // GLFW window pointer is internal to GLViewImpl - we'd need to expose it
    // For now, this is a placeholder that needs cocos2d-x patching
    // (real implementation would call glfwView->getWindow() then ImGui_ImplGlfw_Init)
    return false;
}

void ShutdownImGuiBackend() {
    // ImGui_ImplOpenGL3_Shutdown / ImGui_ImplGlfw_Shutdown handled by Editor::Shutdown
}

void NewFrameImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void RenderImGui() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace luminus
