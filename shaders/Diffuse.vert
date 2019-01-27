#version 150
in vec4 vertex;
in vec3 normal;
in vec4 color;
out vec3 vert;
out vec3 vertNormal;
out vec4 col;
uniform mat4 projMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

void main()
{
    vert = vertex.xyz;
    vertNormal = normalMatrix * normal;
    col = color;
    gl_Position = projMatrix * mvMatrix * vertex;
}
