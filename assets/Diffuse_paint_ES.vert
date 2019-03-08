attribute vec2 vertex;
attribute vec4 color;

varying vec4 col;

uniform mat4 projMatrix;

void main()
{
    col = color;
    gl_Position = projMatrix * vec4(vertex, 0, 1);
}
