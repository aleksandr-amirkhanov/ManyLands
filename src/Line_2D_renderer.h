#pragma once

// glm
#include <glm/glm.hpp>
// OpenGL
#if defined(USE_GL_ES3)
#include <GLES3/gl3.h>  // Use GL ES 3
#else
#include <GL/gl3w.h>
#endif
// std
#include <vector>
// Local
#include "Line_2D.h"

class Line_2D_renderer
{
private:
    struct Array_data
    {
        glm::vec2 vert;
        glm::vec4 color;
    };

public:
    Line_2D_renderer();
    ~Line_2D_renderer();

    void add_line(Line_2D line);
    void add_lines(std::vector<Line_2D> lines);
    void draw_object();

private:
    std::vector<Array_data> data_vector;
    std::vector<GLuint> indices;

    GLuint array_buff_id_;
    GLuint index_buff_id_;
};
