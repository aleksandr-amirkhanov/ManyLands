#version 150
#extension GL_ARB_explicit_attrib_location: enable
#extension GL_ARB_separate_shader_objects: enable

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec4 col;
layout(location = 3) in vec4 viewSpace;

out vec4 fragColor;

uniform vec3 lightPos;
uniform vec2 fogRange;

const vec3 specColor = vec3(0.3, 0.3, 0.3);
const float ambientCoef = 0.3;
const float shininess = 4.0;

void main()
{
    fragColor = vec4(0, 0, 0, 0.5);
}
