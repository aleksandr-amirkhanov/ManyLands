#version 150
#extension GL_ARB_explicit_attrib_location: enable
#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_explicit_uniform_location: enable

layout(location = 4) in vec4 vertex;
layout(location = 5) in vec3 normal;
layout(location = 6) in vec4 color;

out vec3 vert;
out vec3 vertNormal;
out vec4 col;
out vec4 viewSpace;

layout(location = 7) uniform mat4 projMatrix;
layout(location = 8) uniform mat4 mvMatrix;
layout(location = 9) uniform mat3 normalMatrix;

void main()
{
    vert = vertex.xyz;
    vertNormal = normalMatrix * normal;
    col = color;
    viewSpace = mvMatrix * vec4(vertex);

    gl_Position = projMatrix * mvMatrix * vertex;
}
