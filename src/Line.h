#pragma once

// glm
#include <glm/glm.hpp>
// std
#include <vector>

struct Line_point
{
    Line_point(glm::vec2 pos, glm::float32 width, glm::vec4 color)
    {
        this->pos   = pos;
        this->width = width;
        this->color = color;
    }

    glm::vec2    pos;
    glm::float32 width;
    glm::vec4    color;
};

typedef std::vector<Line_point> Line_strip;
