#pragma once

// Local
#include "Base_shader.h"
#include "Geometry_engine.h"
// std
#include <memory>
#include <vector>
// glm
#include <glm/glm.hpp>

//******************************************************************************
// Screen shapes
//******************************************************************************

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

//******************************************************************************
// Screen_shader
//******************************************************************************

class Screen_shader : public Base_shader
{
public:
    struct Data_array
    {
        glm::vec2 vert;
        glm::vec4 color;
    };
    typedef Geometry_engine<Data_array> Screen_geometry;

    std::unique_ptr<Screen_geometry>
        create_geometry(const Line_strip& strip);

    void draw_geometry(const std::unique_ptr<Screen_geometry>& geom);

    void initialize() override;

    GLuint program_id,
           proj_mat_id,
           vertex_attrib_id,
           color_attrib_id;
};
