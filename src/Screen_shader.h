#pragma once

// Local
#include "Base_shader.h"
#include "Geometry_engine.h"
#include "Line.h"
// sdl
#include <memory>

class Screen_shader : public Base_shader
{
public:
    struct Data_array
    {
        glm::vec2 vert;
        glm::vec4 color;
    };
    typedef Geometry_engine<Data_array> Line_geometry;

    std::unique_ptr<Line_geometry>
        create_geometry(const Line_strip& strip);

    void draw_geometry(const std::unique_ptr<Line_geometry>& geom);

    void initialize() override;

    GLuint program_id,
           proj_mat_id,
           vertex_attrib_id,
           color_attrib_id;
};
