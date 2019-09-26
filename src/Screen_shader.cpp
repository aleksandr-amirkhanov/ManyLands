#include "Screen_shader.h"
// Local
#include "Consts.h"
// glm
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

//******************************************************************************
// initialize
//******************************************************************************

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

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
}

//******************************************************************************
// append_to_geometry
//******************************************************************************

void Screen_shader::append_to_geometry(Screen_geometry& geom,
                                       const Line_strip& strip)
{
    auto ind_disp = geom.data_array.size();

    for(auto current = strip.begin(); current != strip.end(); ++current)
    {
        auto next = std::next(current);

        auto calc_norm = [](const Line_point& p1,
                            const Line_point& p2,
                            float width)
        {
            auto norm = glm::rotate(p2.pos - p1.pos,
                                    static_cast<float>(PI / 2));
            return 0.5f * width * glm::normalize(norm);
        };
        
        if(next != strip.end())
        {
            auto norm = calc_norm(*current, *next, current->width);

            Data_array elem[4];

            elem[0].vert = current->pos + norm;
            elem[0].color = current->color;

            elem[1].vert = current->pos - norm;
            elem[1].color = current->color;

            elem[2].vert = next->pos + norm;
            elem[2].color = next->color;

            elem[3].vert = next->pos - norm;
            elem[3].color = next->color;

            geom.data_array.push_back(elem[0]);
            geom.data_array.push_back(elem[1]);
            geom.data_array.push_back(elem[2]);
            geom.data_array.push_back(elem[3]);
        }
    }

    for(size_t i = 0; i < strip.size() - 1; ++i)
    {
        geom.indices.push_back(static_cast<GLint>(ind_disp + i * 4    ));
        geom.indices.push_back(static_cast<GLint>(ind_disp + i * 4 + 2));
        geom.indices.push_back(static_cast<GLint>(ind_disp + i * 4 + 1));
        geom.indices.push_back(static_cast<GLint>(ind_disp + i * 4 + 1));
        geom.indices.push_back(static_cast<GLint>(ind_disp + i * 4 + 2));
        geom.indices.push_back(static_cast<GLint>(ind_disp + i * 4 + 3));
    }
}

//******************************************************************************
// append_to_geometry
//******************************************************************************

void Screen_shader::append_to_geometry(Screen_geometry& geom,
                                       const Rectangle& rect)
{
    auto ind_disp = geom.data_array.size();

    // Create vertices

    Data_array elem[4];

    // The color is the same for all vertices
    for(int i = 0; i < 4; ++i) elem[i].color = rect.color();

    elem[0].vert = glm::vec2(rect.left(),  rect.bottom());
    elem[1].vert = glm::vec2(rect.left(),  rect.top()   );
    elem[2].vert = glm::vec2(rect.right(), rect.top()   );
    elem[3].vert = glm::vec2(rect.right(), rect.bottom());

    for(int i = 0; i < 4; ++i) geom.data_array.push_back(elem[i]);

    // Fill indices

    geom.indices.push_back(static_cast<GLuint>(ind_disp + 0));
    geom.indices.push_back(static_cast<GLuint>(ind_disp + 1));
    geom.indices.push_back(static_cast<GLuint>(ind_disp + 2));
    geom.indices.push_back(static_cast<GLuint>(ind_disp + 2));
    geom.indices.push_back(static_cast<GLuint>(ind_disp + 3));
    geom.indices.push_back(static_cast<GLuint>(ind_disp + 0));
}

//******************************************************************************
// append_to_geometry
//******************************************************************************

void Screen_shader::append_to_geometry(Screen_geometry& geom,
                                       const Triangle& triangle)
{
    auto ind_disp = geom.data_array.size();

    // Create vertices

    Data_array elem[3];

    // The color is the same for all vertices
    for(int i = 0; i < 3; ++i) elem[i].color = triangle.color;

    elem[0].vert = triangle.v1;
    elem[1].vert = triangle.v2;
    elem[2].vert = triangle.v3;

    for(int i = 0; i < 4; ++i) geom.data_array.push_back(elem[i]);

    // Fill indices

    geom.indices.push_back(static_cast<GLint>(ind_disp + 0));
    geom.indices.push_back(static_cast<GLint>(ind_disp + 1));
    geom.indices.push_back(static_cast<GLint>(ind_disp + 2));
}

//******************************************************************************
// draw_geometry
//******************************************************************************

void Screen_shader::draw_geometry(const Screen_geometry& geom)
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, geom.array_buff_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom.index_buff_id);

    glEnableVertexAttribArray(vertex_attrib_id);
    glEnableVertexAttribArray(color_attrib_id );

    GLsizei stride = sizeof(Data_array);
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

    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(geom.indices.size()),
                   GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(vertex_attrib_id);
    glDisableVertexAttribArray(color_attrib_id );
}
