#pragma once

#include <glm/glm.hpp>
#include <vector>

// Internal representation of a mesh. Currently it is very similar to the .OBJ
// format
struct Mesh
{
    struct Vertex
    {
        Vertex(size_t vert_id, size_t norm_id)
            : vertex_id(vert_id)
            , normal_id(norm_id)
        {
        }
        size_t vertex_id;
        size_t normal_id;
    };
    struct Object
    {
        typedef std::vector<Vertex> FaceType;
        std::vector<FaceType> faces;
    };

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> colors;
    std::vector<Object> objects;
};
