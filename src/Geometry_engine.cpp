#include "Geometry_engine.h"
// Local
#include "Mesh.h"

Geometry_engine::Geometry_engine()
{
}

Geometry_engine::~Geometry_engine()
{
    glDeleteBuffers(1, &array_buff_id_);
    glDeleteBuffers(1, &index_buff_id_);
}

Geometry_engine::Geometry_engine(const Mesh& m)
    : Geometry_engine()
{
    create(m);
}

void Geometry_engine::create(const Mesh& m)
{
    // Generating buffers
    glGenBuffers(1, &array_buff_id_);
    glGenBuffers(1, &index_buff_id_);

    vnc_vector.clear();
    indices.clear();
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
                        vnc_vector.push_back(vnc);
                    }
                    // Vertex 2
                    {
                        Array_data vnc;
                        vnc.vert = glm::vec4(m.vertices[f[i + 1].vertex_id], 1);
                        vnc.norm = m.normals[f[i + 1].normal_id];
                        vnc.color = c;
                        vnc_vector.push_back(vnc);
                    }
                    // Vertex 3
                    {
                        Array_data vnc;
                        vnc.vert = glm::vec4(m.vertices[f[i + 2].vertex_id], 1);
                        vnc.norm = m.normals[f[i + 2].normal_id];
                        vnc.color = c;
                        vnc_vector.push_back(vnc);
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
                        vnc_vector.push_back(vnc);
                    }
                }

                indices.push_back(ind);         // Vertex 1
                indices.push_back(ind + i + 1); // Vertex 2
                indices.push_back(ind + i + 2); // Vertex 3
            }

            ind += num_verts;
        }
    }

    // Allocating buffers
    glBindBuffer(GL_ARRAY_BUFFER, array_buff_id_);
	glBufferData(GL_ARRAY_BUFFER, vnc_vector.size() * sizeof(Array_data), &vnc_vector[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buff_id_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
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

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}
