attribute vec4 vertex;
attribute vec3 normal;
attribute vec4 color;

varying vec3 vert;
varying vec3 vertNormal;
varying vec4 col;
varying vec4 viewSpace;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

void main()
{
    vert = vertex.xyz;
    vertNormal = normalMatrix * normal;
    col = color;
	viewSpace = mvMatrix * vec4(vertex);
	
    gl_Position = projMatrix * mvMatrix * vertex;
}
