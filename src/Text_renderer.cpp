#include "Text_renderer.h"
// ImGui
#include "imgui.h"

//******************************************************************************
// render
//******************************************************************************
void Text_renderer::render(int width, int height)
{
    for(auto& t : text_array_)
    {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::Begin(
       "BCKGND",
       0,
       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
          ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
          ImGuiWindowFlags_NoFocusOnAppearing |
          ImGuiWindowFlags_NoBringToFrontOnFocus |
          ImGuiWindowFlags_NoBackground);   
   
        ImGui::GetWindowDrawList()->AddText(
           NULL,
           0.0f,
           ImVec2(t.pos.x, height - t.pos.y),
           ImColor(0.0f, 0.0f, 0.0f, 1.0f),
           t.text.c_str());
        ImGui::End();    
    }
}

//******************************************************************************
// add_text
//******************************************************************************

void Text_renderer::add_text(
    const Base_renderer::Region& region,
    const glm::vec2& pos,
    const std::string& text)
{
    text_array_.emplace_back(
        glm::vec2(region.left() + pos.x, region.bottom() + pos.y), text);
}

//******************************************************************************
// clear
//******************************************************************************

void Text_renderer::clear()
{
    text_array_.clear();
}
