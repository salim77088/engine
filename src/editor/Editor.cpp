#include "Editor.h"
#include "../core/Engine.h"
#include "../core/SceneManager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <cstdarg>

namespace luminus {

Editor& Editor::Instance() { static Editor i; return i; }

bool Editor::Init() {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = "luminus_editor.ini";
    ImGui::StyleColorsDark();

    // Apply Luminus branding to the theme
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg]       = ImVec4(0.10f, 0.10f, 0.13f, 1.0f);
    style.Colors[ImGuiCol_TitleBg]        = ImVec4(0.18f, 0.20f, 0.28f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive]  = ImVec4(0.25f, 0.30f, 0.45f, 1.0f);
    style.Colors[ImGuiCol_Button]         = ImVec4(0.22f, 0.30f, 0.50f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered]  = ImVec4(0.35f, 0.45f, 0.65f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive]   = ImVec4(0.45f, 0.55f, 0.75f, 1.0f);
    style.Colors[ImGuiCol_Header]         = ImVec4(0.20f, 0.25f, 0.40f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered]  = ImVec4(0.30f, 0.40f, 0.60f, 1.0f);
    style.Colors[ImGuiCol_FrameBg]        = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);

    return true;
}

void Editor::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Editor::LogToConsole(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (consoleLen_ + (int)strlen(buf) + 2 < (int)sizeof(consoleBuffer_)) {
        strcat(consoleBuffer_, buf);
        strcat(consoleBuffer_, "\n");
        consoleLen_ += strlen(buf) + 1;
    }
}

void Editor::Render() {
    if (!visible_) return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    RenderMainMenuBar();
    RenderToolbar();
    if (showOutliner_)  RenderOutliner();
    if (showInspector_) RenderInspector();
    if (showConsole_)   RenderConsole();
    if (showAssets_)    RenderAssetBrowser();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::RenderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene"))   {}
            if (ImGui::MenuItem("Open Scene...")){}
            if (ImGui::MenuItem("Save Scene"))  {
                Engine::Instance().Scenes().SaveToFile(Engine::Instance().Scenes().GetCurrentName() + ".json");
            }
            if (ImGui::MenuItem("Exit", "Esc")) Engine::Instance().Quit();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Outliner",  nullptr, &showOutliner_);
            ImGui::MenuItem("Inspector", nullptr, &showInspector_);
            ImGui::MenuItem("Console",   nullptr, &showConsole_);
            ImGui::MenuItem("Assets",    nullptr, &showAssets_);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            ImGui::TextDisabled("Luminus Engine v1.0.0");
            ImGui::TextDisabled("Built on Cocos2d-x v4 (MIT)");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Editor::RenderToolbar() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 40));
    if (ImGui::Begin("##Toolbar", nullptr, flags)) {
        if (ImGui::Button("Play"))      LogToConsole("[Editor] Play pressed");
        if (ImGui::Button("Pause"))     LogToConsole("[Editor] Pause pressed");
        if (ImGui::Button("Stop"))      LogToConsole("[Editor] Stop pressed");
        ImGui::SameLine();
        if (ImGui::Button("Add Sprite")) {
            Entity e;
            e.name = "sprite_" + std::to_string(Engine::Instance().Scenes().GetCurrent().entities.size() + 1);
            e.x = Engine::Instance().GetConfig().width / 2;
            e.y = Engine::Instance().GetConfig().height / 2;
            Engine::Instance().Scenes().AddEntity(e);
        }
        if (ImGui::Button("Add Text"))  {
            Entity e;
            e.name = "text_" + std::to_string(Engine::Instance().Scenes().GetCurrent().entities.size() + 1);
            e.type = "text";
            e.text = "Hello Luminus";
            e.x = Engine::Instance().GetConfig().width / 2 - 80;
            e.y = Engine::Instance().GetConfig().height / 2;
            Engine::Instance().Scenes().AddEntity(e);
        }
        if (ImGui::Button("Save")) {
            Engine::Instance().Scenes().SaveToFile("examples/edited.scene.json");
            LogToConsole("[Editor] Scene saved");
        }
    }
    ImGui::End();
}

void Editor::RenderOutliner() {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight() + 40), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, ImGui::GetIO().DisplaySize.y - 100), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Outliner", &showOutliner_)) {
        auto& scene = Engine::Instance().Scenes().GetCurrent();
        ImGui::Text("Scene: %s", Engine::Instance().Scenes().GetCurrentName().c_str());
        ImGui::Separator();
        for (size_t i = 0; i < scene.entities.size(); i++) {
            const auto& e = scene.entities[i];
            char label[256];
            snprintf(label, sizeof(label), "%s [%s]###%d", e.name.c_str(), e.type.c_str(), (int)i);
            if (ImGui::Selectable(label, selected_ == (int)i)) {
                selected_ = (int)i;
            }
        }
    }
    ImGui::End();
}

void Editor::RenderInspector() {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 320, ImGui::GetFrameHeight() + 40), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, ImGui::GetIO().DisplaySize.y - 100), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Inspector", &showInspector_)) {
        if (selected_ < 0 || selected_ >= (int)Engine::Instance().Scenes().GetCurrent().entities.size()) {
            ImGui::TextDisabled("Select an entity to inspect");
            ImGui::End();
            return;
        }
        Entity& e = Engine::Instance().Scenes().GetCurrent().entities[selected_];
        char nameBuf[128];
        snprintf(nameBuf, sizeof(nameBuf), "%s", e.name.c_str());
        ImGui::InputText("Name", nameBuf, sizeof(nameBuf));
        e.name = nameBuf;
        ImGui::Text("Type: %s", e.type.c_str());
        ImGui::Separator();
        ImGui::DragFloat("X", &e.x, 1.0f);
        ImGui::DragFloat("Y", &e.y, 1.0f);
        ImGui::DragFloat("Z", &e.z, 1.0f);
        ImGui::DragFloat("Width",  &e.width,  1.0f);
        ImGui::DragFloat("Height", &e.height, 1.0f);
        ImGui::DragFloat("Rotation", &e.rotation, 1.0f, 0.0f, 360.0f);
        ImGui::DragFloat("Scale",    &e.scale, 0.01f, 0.1f, 10.0f);
        ImGui::SliderFloat("Opacity", &e.opacity, 0.0f, 1.0f);
        ImGui::Checkbox("Visible", &e.visible);
        ImGui::Separator();
        if (e.type == "text") {
            char txt[512];
            snprintf(txt, sizeof(txt), "%s", e.text.c_str());
            ImGui::InputTextMultiline("Text", txt, sizeof(txt), ImVec2(-1, 80));
            e.text = txt;
            ImGui::DragInt("Font Size", &e.fontSize, 1, 8, 256);
        }
        char colorBuf[32];
        snprintf(colorBuf, sizeof(colorBuf), "%s", e.color.c_str());
        ImGui::InputText("Color", colorBuf, sizeof(colorBuf));
        e.color = colorBuf;
        ImGui::Separator();
        if (ImGui::Button("Delete Entity")) {
            Engine::Instance().Scenes().RemoveEntity(e.name);
            selected_ = -1;
        }
    }
    ImGui::End();
}

void Editor::RenderConsole() {
    ImGui::SetNextWindowPos(ImVec2(280, ImGui::GetIO().DisplaySize.y - 200), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 600, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Console", &showConsole_)) {
        if (ImGui::Button("Clear")) { consoleBuffer_[0] = 0; consoleLen_ = 0; }
        ImGui::SameLine();
        ImGui::TextDisabled("(%d bytes)", consoleLen_);
        ImGui::Separator();
        ImGui::BeginChild("console_scroll");
        ImGui::TextUnformatted(consoleBuffer_);
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();
    }
    ImGui::End();
}

void Editor::RenderAssetBrowser() {
    if (!ImGui::Begin("Assets", &showAssets_)) { ImGui::End(); return; }
    ImGui::TextDisabled("Asset browser (TODO)");
    ImGui::End();
}

} // namespace luminus
