#pragma once

// gl3w
#if defined(USE_GL_ES3)
#include <GLES3/gl3.h>  // Use GL ES 3
#else
#include <GL/gl3w.h>
#endif
// std
#include <vector>

#include <utility>

// The class provides a convenient way to draw a mesh objects
template<class TArray_data>
class Geometry_engine
{
public:
    Geometry_engine();
    Geometry_engine(const Geometry_engine& other);
    Geometry_engine(Geometry_engine&& other) noexcept;
    Geometry_engine& operator=(const Geometry_engine& other);
    Geometry_engine& operator=(Geometry_engine&& other) noexcept;

    ~Geometry_engine();

    void init_buffers();

    std::vector<TArray_data> data_array; // vertices + normals + colors
    std::vector<GLuint> indices;

    GLuint array_buff_id;
    GLuint index_buff_id;
    GLuint vao;
};

template<class TArray_data>
Geometry_engine<TArray_data>::Geometry_engine()
{
    // Generate vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Generating buffers
    glGenBuffers(1, &array_buff_id);
    glGenBuffers(1, &index_buff_id);
}

template<class TArray_data>
Geometry_engine<TArray_data>::~Geometry_engine()
{
    glDeleteBuffers(1, &array_buff_id);
    glDeleteBuffers(1, &index_buff_id);
}

template<class TArray_data>
void Geometry_engine<TArray_data>::init_buffers()
{
    // Allocating buffers
    glBindBuffer(GL_ARRAY_BUFFER, array_buff_id);
    glBufferData(
        GL_ARRAY_BUFFER,
        data_array.size() * sizeof(TArray_data),
        &data_array[0],
        GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buff_id);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(GLuint),
        &indices[0],
        GL_STATIC_DRAW);
}

template<class TArray_data>
Geometry_engine<TArray_data>::Geometry_engine(const Geometry_engine& other)
    : data_array(other.data_array)
    , indices(other.indices)
{
    init_buffers();
}

template<class TArray_data>
Geometry_engine<TArray_data>::Geometry_engine(Geometry_engine&& other) noexcept
    : data_array(std::exchange(other.data_array, std::vector<TArray_data>()))
    , indices(std::exchange(other.indices, std::vector<GLuint>()))
    , array_buff_id(std::exchange(other.array_buff_id, GLuint(0)))
    , index_buff_id(std::exchange(other.index_buff_id, GLuint(0)))
{}

template<class TArray_data>
Geometry_engine<TArray_data>& Geometry_engine<TArray_data>::operator=(
    const Geometry_engine& other)
{
    return *this = Geometry_engine(other);
}

template<class TArray_data>
Geometry_engine<TArray_data>& Geometry_engine<TArray_data>::operator=(
    Geometry_engine&& other) noexcept
{
    std::swap(data_array, other.data_array);
    std::swap(indices, other.indices);
    std::swap(array_buff_id, other.array_buff_id);
    std::swap(index_buff_id, other.index_buff_id);
    return *this;
}
