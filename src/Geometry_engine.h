#pragma once

// glm
#include <glm/glm.hpp>
// gl3w
#if defined(USE_GL_ES3)
#include <GLES3/gl3.h>  // Use GL ES 3
#else
#include <GL/gl3w.h>
#endif
// std
#include <vector>

struct Mesh;

// The class provides a convenient way to draw a mesh objects
class Geometry_engine
{
private:
    struct Array_data
    {
        glm::vec4 vert;
        glm::vec3 norm;
        glm::vec4 color;
    };
    Geometry_engine() = delete;
    void init_buffers();

public:
    Geometry_engine(const Mesh& m);
    Geometry_engine(const Geometry_engine& other);
    Geometry_engine(Geometry_engine&& other) noexcept;
    Geometry_engine& operator=(const Geometry_engine& other);
    Geometry_engine& operator=(Geometry_engine&& other) noexcept;

    ~Geometry_engine();
    void create(const Mesh& m);
    void draw_object();

private:
    std::vector<Array_data> vnc_vector_; // vertices + normals + colors
    std::vector<GLuint> indices_;

    GLuint array_buff_id_;
    GLuint index_buff_id_;
};
