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
// Screen_shader
//******************************************************************************

class Screen_shader : public Base_shader
{
public:
    // Screen shapes
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

    struct Rectangle
    {
    public:
        Rectangle(float left,
                  float bottom,
                  float right,
                  float top,
                  glm::vec4 color)
        {
            left_   = left;
            bottom_ = bottom;
            right_  = right;
            top_    = top;
            color_  = color;
        }
        float left()   const { return left_;   }
        float bottom() const { return bottom_; }
        float right()  const { return right_;  }
        float top()    const { return top_;    }

        const glm::vec4& color() const { return color_; }

    private:
        float left_, bottom_, right_, top_;
        glm::vec4 color_;
    };

    struct Triangle
    {
        glm::vec2 v1, v2, v3;
        glm::vec4 color;
    };

    struct Data_array
    {
        glm::vec2 vert;
        glm::vec4 color;
    };
    typedef Geometry_engine<Data_array> Screen_geometry;

    void append_to_geometry(Screen_geometry& geom, const Line_strip& strip);
    void append_to_geometry(Screen_geometry& geom, const Rectangle& rect);
    void append_to_geometry(Screen_geometry& geom, const Triangle& triangle);

    // Please do not forget to call the `init_buffers` method before drawing the
    // geometry
    void draw_geometry(const std::unique_ptr<Screen_geometry>& geom);

    void initialize() override;

    GLuint program_id,
           proj_mat_id,
           vertex_attrib_id,
           color_attrib_id;
};
