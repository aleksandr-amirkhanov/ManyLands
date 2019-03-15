#pragma once

// Local
#include "Base_shader.h"
#include "Geometry_engine.h"
#include "Mesh.h"
// sdl
#include <memory>

class Diffuse_shader : public Base_shader
{
public:
    struct Data_array
    {
        glm::vec4 vert;
        glm::vec3 norm;
        glm::vec4 color;
    };

    typedef Geometry_engine<Data_array> Mesh_geometry;

    void initialize() override;

    void append_to_geometry(Mesh_geometry& geom, const Mesh& m);
    void draw_geometry(const std::unique_ptr<Mesh_geometry>& geom);

    GLuint program_id,
           proj_mat_id,
           mv_mat_id,
           normal_mat_id,
           light_pos_id,
           vertex_attrib_id,
           normal_attrib_id,
           color_attrib_id,
           fog_range_id;
};
