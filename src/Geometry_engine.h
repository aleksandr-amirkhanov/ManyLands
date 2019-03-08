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

public:
    std::vector<TArray_data> vnc_vector_; // vertices + normals + colors
    std::vector<GLuint> indices_;

    GLuint array_buff_id_;
    GLuint index_buff_id_;
};

template<class TArray_data>
Geometry_engine<TArray_data>::Geometry_engine()
{
    // Generating buffers
    glGenBuffers(1, &array_buff_id_);
    glGenBuffers(1, &index_buff_id_);
}

template<class TArray_data>
Geometry_engine<TArray_data>::~Geometry_engine()
{
    glDeleteBuffers(1, &array_buff_id_);
    glDeleteBuffers(1, &index_buff_id_);
}

template<class TArray_data>
void Geometry_engine<TArray_data>::init_buffers()
{
    // Allocating buffers
    glBindBuffer(GL_ARRAY_BUFFER, array_buff_id_);
    glBufferData(
        GL_ARRAY_BUFFER,
        vnc_vector_.size() * sizeof(TArray_data),
        &vnc_vector_[0],
        GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buff_id_);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices_.size() * sizeof(GLuint),
        &indices_[0],
        GL_STATIC_DRAW);
}

template<class TArray_data>
Geometry_engine<TArray_data>::Geometry_engine(const Geometry_engine& other)
    : vnc_vector_(other.vnc_vector_)
    , indices_(other.indices_)
{
    init_buffers();
}

template<class TArray_data>
Geometry_engine<TArray_data>::Geometry_engine(Geometry_engine&& other) noexcept
    : vnc_vector_(std::exchange(other.vnc_vector_, std::vector<TArray_data>()))
    , indices_(std::exchange(other.indices_, std::vector<GLuint>()))
    , array_buff_id_(std::exchange(other.array_buff_id_, GLuint(0)))
    , index_buff_id_(std::exchange(other.index_buff_id_, GLuint(0)))
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
    std::swap(vnc_vector_, other.vnc_vector_);
    std::swap(indices_, other.indices_);
    std::swap(array_buff_id_, other.array_buff_id_);
    std::swap(index_buff_id_, other.index_buff_id_);
    return *this;
}
