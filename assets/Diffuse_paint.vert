#version 150
#extension GL_ARB_explicit_attrib_location: enable
#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_explicit_uniform_location: enable

layout(location = 10) in vec2 vertex;
layout(location = 1) in vec4 color;
out vec4 col;
layout(location = 2) uniform mat4 projMatrix;

void main()
{
    col = color;
    gl_Position = projMatrix * vec4(vertex, 0, 1);
}
