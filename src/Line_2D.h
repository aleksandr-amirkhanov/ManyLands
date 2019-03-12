#pragma once

// glm
#include <glm/glm.hpp>
// std
#include <vector>

// TODO: rename this file from 'Line_2D.h' to 'Line.h'

/*struct Line_2D
{
    glm::vec2    start_pos, end_pos;
    glm::float32 width;
    glm::vec4    color;
};*/

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
