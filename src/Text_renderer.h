#pragma once

// local
#include "Base_renderer.h"
// glm
#include <glm/glm.hpp>
// std
#include <string>
#include <vector>

//******************************************************************************
// Text_renderer
//
// This is a very simple text renderer that uses ImGui backend
//******************************************************************************

class Text_renderer
{
private:
    struct Text_data
    {
        Text_data(glm::vec2 pos, std::string text)
        {
            this->pos  = pos;
            this->text = text;
        }

        glm::vec2 pos;
        std::string text;
    };
    
public:
    void render(int width, int height);
    void add_text(
        const Base_renderer::Region& region,
        const glm::vec2& pos,
        const std::string& text);
    void clear();

private:
    std::vector<Text_data> text_array_;
};
