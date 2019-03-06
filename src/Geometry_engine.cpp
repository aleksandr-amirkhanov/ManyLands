#include "Geometry_engine.h"
// Local
#include "Mesh.h"

#include <utility>

Geometry_engine::~Geometry_engine()
{
    glDeleteBuffers(1, &array_buff_id_);
    glDeleteBuffers(1, &index_buff_id_);
}

void Geometry_engine::init_buffers()
{
    // Generating buffers
    glGenBuffers(1, &array_buff_id_);
    glGenBuffers(1, &index_buff_id_);
    // Allocating buffers
    glBindBuffer(GL_ARRAY_BUFFER, array_buff_id_);
    glBufferData(
        GL_ARRAY_BUFFER,
        vnc_vector_.size() * sizeof(Array_data),
        &vnc_vector_[0],
        GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buff_id_);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices_.size() * sizeof(GLuint),
        &indices_[0],
        GL_STATIC_DRAW);
}

Geometry_engine::Geometry_engine(const Mesh& m)
{
    create(m);
    init_buffers();
}

Geometry_engine::Geometry_engine(const Geometry_engine& other)
    : vnc_vector_(other.vnc_vector_)
    , indices_(other.indices_)
{
    init_buffers();
}

Geometry_engine::Geometry_engine(Geometry_engine&& other) noexcept
    : vnc_vector_(std::exchange(other.vnc_vector_, std::vector<Array_data>()))
    , indices_(std::exchange(other.indices_, std::vector<GLuint>()))
    , array_buff_id_(std::exchange(other.array_buff_id_, GLuint(0)))
    , index_buff_id_(std::exchange(other.index_buff_id_, GLuint(0)))
{}

Geometry_engine& Geometry_engine::operator=(const Geometry_engine& other)
{
    return *this = Geometry_engine(other);
}

Geometry_engine& Geometry_engine::operator=(Geometry_engine&& other) noexcept
{
    std::swap(vnc_vector_, other.vnc_vector_);
    std::swap(indices_, other.indices_);
    std::swap(array_buff_id_, other.array_buff_id_);
    std::swap(index_buff_id_, other.index_buff_id_);
    return *this;
}

void Geometry_engine::create(const Mesh& m)
{
    vnc_vector_.clear();
    indices_.clear();
    GLuint ind = 0;

    for(size_t i = 0; i < m.objects.size(); ++i)
    {
        const auto& obj = m.objects[i];
        const auto& c = m.colors[i];

        for(auto const& f : obj.faces)
        {
            size_t num_verts = f.size();
            size_t num_triangles = num_verts - 2;

            for(size_t i = 0; i < num_triangles; ++i)
            {
                if(i == 0)
                {
                    // If the current triangle is first in the face we add three
                    // pairs of vertices and normals to the array

                    // Vertex 1
                    {
                        Array_data vnc;
                        vnc.vert = glm::vec4(m.vertices[f[0].vertex_id], 1);
                        vnc.norm = m.normals[f[0].normal_id];
                        vnc.color = c;
                        vnc_vector_.push_back(vnc);
                    }
                    // Vertex 2
                    {
                        Array_data vnc;
                        vnc.vert = glm::vec4(m.vertices[f[i + 1].vertex_id], 1);
                        vnc.norm = m.normals[f[i + 1].normal_id];
                        vnc.color = c;
                        vnc_vector_.push_back(vnc);
                    }
                    // Vertex 3
                    {
                        Array_data vnc;
                        vnc.vert = glm::vec4(m.vertices[f[i + 2].vertex_id], 1);
                        vnc.norm = m.normals[f[i + 2].normal_id];
                        vnc.color = c;
                        vnc_vector_.push_back(vnc);
                    }
                }
                else
                {
                    // If the current triangle is not the first in the face it
                    // is enough to add only the one pair of vertices and
                    // normals to the array

                    // Vertex 3
                    {
                        Array_data vnc;
                        vnc.vert = glm::vec4(m.vertices[f[i + 2].vertex_id], 1);
                        vnc.norm = m.normals[f[i + 2].normal_id];
                        vnc.color = c;
                        vnc_vector_.push_back(vnc);
                    }
                }

                indices_.push_back(ind);         // Vertex 1
                indices_.push_back(ind + i + 1); // Vertex 2
                indices_.push_back(ind + i + 2); // Vertex 3
            }

            ind += num_verts;
        }
    }
}

void Geometry_engine::draw_object()
{
    glBindBuffer(GL_ARRAY_BUFFER, array_buff_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buff_id_);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    GLsizei stride = sizeof(Array_data);
    void* ptr1 = reinterpret_cast<void*>(4 * sizeof(GLfloat));
    void* ptr2 = reinterpret_cast<void*>(7 * sizeof(GLfloat));
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, ptr1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, ptr2);

    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}
