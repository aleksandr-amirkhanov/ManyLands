#version 150
in highp vec3 vert;
in highp vec3 vertNormal;
in highp vec4 col;
out highp vec4 fragColor;
uniform highp vec3 lightPos;

void main()
{
  highp vec3 L = normalize(lightPos - vert);
  highp float NL = max(dot(normalize(vertNormal), L), 0.0);
  highp vec4 c = clamp(col * NL, 0.0, 1.0);
  fragColor = c;
}
