#version 150
#extension GL_ARB_explicit_attrib_location: enable
#extension GL_ARB_separate_shader_objects: enable

layout(location = 3) in highp vec4 col;
out highp vec4 fragColor;

void main()
{
    fragColor = col;
}
