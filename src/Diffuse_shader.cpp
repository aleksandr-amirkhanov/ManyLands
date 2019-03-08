#include "Diffuse_shader.h"
// glm
#include <glm/glm.hpp>

void Diffuse_shader::initialize()
{
#ifdef __EMSCRIPTEN__
    program_id = load_shaders(
        "assets/Diffuse_ES.vert",
        "assets/Diffuse_ES.frag");
#else
    program_id = load_shaders(
        "assets/Diffuse.vert",
        "assets/Diffuse.frag");
#endif

    //glUseProgram(program_id);
    proj_mat_id   = glGetUniformLocation(program_id,  "projMatrix");
    mv_mat_id     = glGetUniformLocation(program_id,    "mvMatrix");
    normal_mat_id = glGetUniformLocation(program_id,"normalMatrix");
    light_pos_id  = glGetUniformLocation(program_id,    "lightPos");

    vertex_attrib_id = glGetAttribLocation(program_id, "vertex");
    normal_attrib_id = glGetAttribLocation(program_id, "normal");
    color_attrib_id  = glGetAttribLocation(program_id,  "color");
}

std::unique_ptr<Diffuse_shader::Mesh_geometry>
Diffuse_shader::create_mesh_geometry(const Mesh& m)
{
    std::unique_ptr<Mesh_geometry> geom = std::make_unique<Mesh_geometry>();
    geom->data_array.clear();
    geom->indices.clear();
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
                        Mesh_array vnc;
                        vnc.vert = glm::vec4(m.vertices[f[0].vertex_id], 1);
                        vnc.norm = m.normals[f[0].normal_id];
                        vnc.color = c;
                        geom->data_array.push_back(vnc);
                    }
                    // Vertex 2
                    {
                        Mesh_array vnc;
                        vnc.vert = glm::vec4(m.vertices[f[i + 1].vertex_id], 1);
                        vnc.norm = m.normals[f[i + 1].normal_id];
                        vnc.color = c;
                        geom->data_array.push_back(vnc);
                    }
                    // Vertex 3
                    {
                        Mesh_array vnc;
                        vnc.vert = glm::vec4(m.vertices[f[i + 2].vertex_id], 1);
                        vnc.norm = m.normals[f[i + 2].normal_id];
                        vnc.color = c;
                        geom->data_array.push_back(vnc);
                    }
                }
                else
                {
                    // If the current triangle is not the first in the face it
                    // is enough to add only the one pair of vertices and
                    // normals to the array

                    // Vertex 3
                    {
                        Mesh_array vnc;
                        vnc.vert = glm::vec4(m.vertices[f[i + 2].vertex_id], 1);
                        vnc.norm = m.normals[f[i + 2].normal_id];
                        vnc.color = c;
                        geom->data_array.push_back(vnc);
                    }
                }

                geom->indices.push_back(ind);         // Vertex 1
                geom->indices.push_back(ind + i + 1); // Vertex 2
                geom->indices.push_back(ind + i + 2); // Vertex 3
            }

            ind += num_verts;
        }
    }

    geom->init_buffers();
    return geom;
}

void Diffuse_shader::draw_mesh_geometry(
    const std::unique_ptr<Mesh_geometry>& geom)
{
    glBindBuffer(GL_ARRAY_BUFFER, geom->array_buff_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->index_buff_id);

    glEnableVertexAttribArray(vertex_attrib_id);
    glEnableVertexAttribArray(normal_attrib_id);
    glEnableVertexAttribArray(color_attrib_id );

    GLsizei stride = sizeof(Mesh_array);
    void* ptr1 = reinterpret_cast<void*>(4 * sizeof(GLfloat));
    void* ptr2 = reinterpret_cast<void*>(7 * sizeof(GLfloat));
    glVertexAttribPointer(vertex_attrib_id,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          stride, 0);
    glVertexAttribPointer(normal_attrib_id,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          stride, ptr1);
    glVertexAttribPointer(color_attrib_id,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          stride,
                          ptr2);

    glDrawElements(GL_TRIANGLES, geom->indices.size(), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(vertex_attrib_id);
    glDisableVertexAttribArray(normal_attrib_id);
    glDisableVertexAttribArray(color_attrib_id );
}
