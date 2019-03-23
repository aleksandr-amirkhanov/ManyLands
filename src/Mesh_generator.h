#pragma once

// Local
#include "Mesh.h"
// glm
#include <glm/glm.hpp>
// boost
#include <boost/numeric/ublas/matrix.hpp>

namespace Mesh_generator
{
Mesh cylinder(
    unsigned int num_verts,
    float start_diameter,
    float end_diameter,
    const glm::vec3& start_point,
    const glm::vec3& end_point,
    const glm::vec4& color);

void cylinder(
    unsigned int num_verts,
    float start_diameter,
    float end_diameter,
    const glm::vec3& start_point,
    const glm::vec3& end_point,
    const glm::vec4& color,
    Mesh& mesh);

Mesh cylinder_v2(
    unsigned int num_verts,
    float start_diameter,
    float end_diameter,
    const glm::vec3& start_point,
    const glm::vec3& end_point,
    const glm::vec3& start_dir,
    const glm::vec3& end_dir,
    const glm::vec4& color);

void cylinder_v2(
    unsigned int num_verts,
    float start_diameter,
    float end_diameter,
    const glm::vec3& start_point,
    const glm::vec3& end_point,
    const glm::vec3& start_dir,
    const glm::vec3& end_dir,
    const glm::vec4& color,
    Mesh& mesh);

Mesh sphere(
    unsigned int segments,
    unsigned int rings,
    float diameter,
    const glm::vec3& position,
    const glm::vec4& color);

void sphere(
    unsigned int segments,
    unsigned int rings,
    float diameter,
    const glm::vec3& position,
    const glm::vec4& color,
    Mesh& mesh);

} // namespace Mesh_generator
