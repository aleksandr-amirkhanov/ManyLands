#version 150
in vec2 vertex;
in vec4 color;
out vec4 col;
uniform mat4 projMatrix;

void main()
{
    col = color;
    gl_Position = projMatrix * vec4(vertex, 0, 1);
}
