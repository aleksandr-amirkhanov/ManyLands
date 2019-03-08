#include "Screen_shader.h"

void Screen_shader::initialize()
{
#ifdef __EMSCRIPTEN__
    program_id = load_shaders("assets/Diffuse_paint_ES.vert",
                              "assets/Diffuse_paint_ES.frag");
#else
    program_id = load_shaders("assets/Diffuse_paint.vert",
                              "assets/Diffuse_paint.frag");
#endif  

    //glUseProgram(program_id);
    proj_mat_id = glGetUniformLocation(program_id, "projMatrix");

    vertex_attrib_id = glGetAttribLocation(program_id, "vertex");
    color_attrib_id  = glGetAttribLocation(program_id,  "color");
}

std::unique_ptr<Screen_shader::Line_geometry>
Screen_shader::create_line_geometry(const Line_2D& line)
{
    std::unique_ptr<Line_geometry> geom = std::make_unique<Line_geometry>();

    geom->data_array.clear();
    geom->indices.clear();

    Line_array d1, d2;

    d1.vert = line.start_pos;
    d1.color = line.color;

    d2.vert = line.end_pos;
    d2.color = line.color;

    geom->data_array.push_back(d1);
    geom->data_array.push_back(d2);

    geom->indices.push_back(0);
    geom->indices.push_back(1);

    // Allocating buffers
    glBindBuffer(GL_ARRAY_BUFFER, geom->array_buff_id);
	glBufferData(GL_ARRAY_BUFFER,
                 geom->data_array.size() * sizeof(Line_array),
                 &geom->data_array[0],
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->index_buff_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 geom->indices.size() * sizeof(GLuint),
                 &geom->indices[0],
                 GL_STATIC_DRAW);

    geom->init_buffers();
    return geom;
}

void Screen_shader::draw_line_geometry(
    const std::unique_ptr<Line_geometry>& geom)
{
    glBindBuffer(GL_ARRAY_BUFFER, geom->array_buff_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->index_buff_id);

    glEnableVertexAttribArray(vertex_attrib_id);
    glEnableVertexAttribArray(color_attrib_id );

    GLsizei stride = sizeof(Line_array);
    void* ptr = reinterpret_cast<void*>(2 * sizeof(GLfloat));
    glVertexAttribPointer(vertex_attrib_id,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          stride,
                          0);
    glVertexAttribPointer(color_attrib_id,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          stride,
                          ptr);

    glDrawElements(GL_LINES, geom->indices.size(), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(vertex_attrib_id);
    glDisableVertexAttribArray(color_attrib_id );
}
