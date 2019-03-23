#include "Mesh_generator.h"
// Local
#include "Consts.h"
// glm
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <cassert>

//******************************************************************************
// cylinder
//******************************************************************************

Mesh Mesh_generator::cylinder(
    unsigned int num_verts,
    float start_diameter,
    float end_diameter,
    glm::vec3 start_point,
    glm::vec3 end_point,
    glm::vec4 color)
{
    Mesh cylinder_mesh;
    cylinder(
        num_verts,
        start_diameter,
        end_diameter,
        start_point,
        end_point,
        color,
        cylinder_mesh);
    return cylinder_mesh;
}

//******************************************************************************
// cylinder
//******************************************************************************

void Mesh_generator::cylinder(
    unsigned int num_verts,
    float start_diameter,
    float end_diameter,
    glm::vec3 start_point,
    glm::vec3 end_point,
    glm::vec4 color,
    Mesh& mesh)
{
    assert(num_verts >= 3);

    mesh.colors.push_back(color);

    auto dir = end_point - start_point;
    float length = glm::length(dir);

    const glm::vec3 up_vec(0.f, 0.f, 1.f);
    auto rotation = glm::rotation(glm::normalize(up_vec), glm::normalize(dir));

    size_t first_vert = mesh.vertices.size();
    size_t first_norm = mesh.normals.size();

    // creating vertices and normals
    for(unsigned int i = 0; i < num_verts + 1; ++i)
    {
        float angle = static_cast<float>(2 * PI * i / num_verts);

        glm::vec3 lower_vert;
        lower_vert[0] = 0.5f * start_diameter * std::cos(angle);
        lower_vert[1] = 0.5f * start_diameter * std::sin(angle);
        lower_vert[2] = 0.0f;

        glm::vec3 upper_vert;
        upper_vert[0] = 0.5f * end_diameter * std::cos(angle);
        upper_vert[1] = 0.5f * end_diameter * std::sin(angle);
        upper_vert[2] = length;

        glm::vec3 normal;
        normal[0] = lower_vert[0];
        normal[1] = lower_vert[1];
        normal[2] = 0;
        normal = glm::normalize(normal);

        mesh.vertices.push_back(rotation * lower_vert + start_point);
        mesh.vertices.push_back(rotation * upper_vert + start_point);
        mesh.normals.push_back(rotation * normal);
        mesh.normals.push_back(rotation * normal);
    }

    Mesh::Object object;
    // creating faces
    for(unsigned int i = 0; i < num_verts; ++i)
    {
        auto vert_shift = first_vert + 2 * i;
        auto norm_shift = first_norm + 2 * i;

        Mesh::Object::FaceType f1;
        f1.emplace_back(Mesh::Vertex(vert_shift, norm_shift));
        f1.emplace_back(Mesh::Vertex(vert_shift + 2, norm_shift + 2));
        f1.emplace_back(Mesh::Vertex(vert_shift + 1, norm_shift + 1));

        Mesh::Object::FaceType f2;
        f2.emplace_back(Mesh::Vertex(vert_shift + 1, norm_shift + 1));
        f2.emplace_back(Mesh::Vertex(vert_shift + 2, norm_shift + 2));
        f2.emplace_back(Mesh::Vertex(vert_shift + 3, norm_shift + 3));

        object.faces.push_back(f1);
        object.faces.push_back(f2);
    }

    mesh.objects.push_back(object);
}

void Mesh_generator::cylinder_v2(
    unsigned int num_verts,
    float start_diameter,
    float end_diameter,
    glm::vec3 start_point,
    glm::vec3 end_point,
    glm::vec3 start_dir,
    glm::vec3 end_dir,
    glm::vec4 color,
    Mesh& mesh)
{
    // avoid very sharp angles
    // FIXME: don't know why 0.1, should be e.g., -0.8????
    if(glm::dot(start_dir, end_dir) < 0.1f)
    {
        return cylinder(
            num_verts,
            start_diameter,
            end_diameter,
            start_point,
            end_point,
            color,
            mesh);
    }
    assert(num_verts >= 3);

    mesh.colors.push_back(color);

    auto dir = end_point - start_point;
    float length = glm::length(dir);

    const glm::vec3 up_vec(0.f, 0.f, 1.f);
    auto rotation = glm::rotation(glm::normalize(up_vec), glm::normalize(dir));

    const size_t first_vert = mesh.vertices.size();
    const size_t first_norm = mesh.normals.size();

    const float s_d = -glm::dot(start_dir, start_point);
    const float e_d = -glm::dot(end_dir, end_point);
    // creating vertices and normals
    for(unsigned int i = 0; i < num_verts + 1; ++i)
    {
        float angle = static_cast<float>(2 * PI * i / num_verts);

        glm::vec3 lower_vert;
        lower_vert[0] = 0.5f * start_diameter * std::cos(angle);
        lower_vert[1] = 0.5f * start_diameter * std::sin(angle);
        lower_vert[2] = 0.0f;

        glm::vec3 upper_vert;
        upper_vert[0] = 0.5f * end_diameter * std::cos(angle);
        upper_vert[1] = 0.5f * end_diameter * std::sin(angle);
        upper_vert[2] = length;

        glm::vec3 normal;
        normal[0] = lower_vert[0];
        normal[1] = lower_vert[1];
        normal[2] = 0;
        normal = glm::normalize(normal);

        // intersect ray with planes bisecting joints
        // then adjust points to the intersection points
        const glm::vec3 s = rotation * lower_vert + start_point;
        const glm::vec3 e = rotation * upper_vert + start_point;
        const glm::vec3 dir = e - s;
        const float t_s =
            -(glm::dot(start_dir, s) + s_d) / glm::dot(start_dir, dir);
        const float t_e =
            -(glm::dot(end_dir, s) + e_d) / glm::dot(end_dir, dir);
        lower_vert = s + dir * t_s;
        upper_vert = s + dir * t_e;

        mesh.vertices.push_back(lower_vert);
        mesh.vertices.push_back(upper_vert);
        mesh.normals.push_back(rotation * normal);
        mesh.normals.push_back(rotation * normal);
    }

    Mesh::Object object;
    // creating faces
    for(unsigned int i = 0; i < num_verts; ++i)
    {
        auto vert_shift = first_vert + 2 * i;
        auto norm_shift = first_norm + 2 * i;

        Mesh::Object::FaceType f1;
        f1.emplace_back(Mesh::Vertex(vert_shift, norm_shift));
        f1.emplace_back(Mesh::Vertex(vert_shift + 2, norm_shift + 2));
        f1.emplace_back(Mesh::Vertex(vert_shift + 1, norm_shift + 1));

        Mesh::Object::FaceType f2;
        f2.emplace_back(Mesh::Vertex(vert_shift + 1, norm_shift + 1));
        f2.emplace_back(Mesh::Vertex(vert_shift + 2, norm_shift + 2));
        f2.emplace_back(Mesh::Vertex(vert_shift + 3, norm_shift + 3));

        object.faces.push_back(f1);
        object.faces.push_back(f2);
    }

    mesh.objects.push_back(object);
}

Mesh Mesh_generator::cylinder_v2(
    unsigned int num_verts,
    float start_diameter,
    float end_diameter,
    glm::vec3 start_point,
    glm::vec3 end_point,
    glm::vec3 start_dir,
    glm::vec3 end_dir,
    glm::vec4 color)
{
    Mesh cylinder_mesh;
    cylinder_v2(
        num_verts,
        start_diameter,
        end_diameter,
        start_point,
        end_point,
        start_dir,
        end_dir,
        color,
        cylinder_mesh);
    return cylinder_mesh;
}

//******************************************************************************
// sphere
//******************************************************************************

Mesh Mesh_generator::sphere(
    unsigned int segments,
    unsigned int rings,
    float diameter,
    glm::vec3 position,
    glm::vec4 color)
{
    Mesh sphere_mesh;
    sphere(segments, rings, diameter, position, color, sphere_mesh);
    return sphere_mesh;
}

//******************************************************************************
// sphere
//******************************************************************************

void Mesh_generator::sphere(
    unsigned int segments,
    unsigned int rings,
    float diameter,
    glm::vec3 position,
    glm::vec4 color,
    Mesh& mesh)
{
    mesh.colors.push_back(color);

    size_t first_vert = mesh.vertices.size();
    size_t first_norm = mesh.normals.size();

    for(unsigned int i = 0; i < segments + 1; ++i)
    {
        for(unsigned int j = 0; j < rings + 1; ++j)
        {
            float alpha = static_cast<float>(2 * PI * i / segments);
            float betta = static_cast<float>(2 * PI * j / rings);

            glm::vec3 vert;
            vert[0] = diameter * std::sin(alpha) * cos(betta);
            vert[1] = diameter * std::sin(alpha) * sin(betta);
            vert[2] = diameter * std::cos(alpha);

            mesh.vertices.push_back(vert + position);
            mesh.normals.push_back(glm::normalize(vert));
        }
    }

    auto vert_index = [&](unsigned int i, unsigned int j) {
        return (first_vert + i * (segments + 1) + j);
    };
    auto norm_index = [&](unsigned int i, unsigned int j) {
        return (first_norm + i * (segments + 1) + j);
    };

    Mesh::Object object;
    // Optimization: resize the face array to its final size
    object.faces.resize(2 * segments * rings);

    for(unsigned int i = 0; i < segments; ++i)
    {
        for(unsigned int j = 0; j < rings; ++j)
        {
            Mesh::Object::FaceType& f1 = object.faces[2 * (i * rings + j)];
            f1.emplace_back(Mesh::Vertex(vert_index(i, j), norm_index(i, j)));
            f1.emplace_back(
                Mesh::Vertex(vert_index(i, j + 1), norm_index(i, j + 1)));
            f1.emplace_back(
                Mesh::Vertex(vert_index(i + 1, j), norm_index(i + 1, j)));

            Mesh::Object::FaceType& f2 = object.faces[2 * (i * rings + j) + 1];
            f2.emplace_back(Mesh::Vertex(
                vert_index(i + 1, j + 1), norm_index(i + 1, j + 1)));
            f2.emplace_back(
                Mesh::Vertex(vert_index(i + 1, j), norm_index(i + 1, j)));
            f2.emplace_back(
                Mesh::Vertex(vert_index(i, j + 1), norm_index(i, j + 1)));
        }
    }

    mesh.objects.push_back(object);
}
