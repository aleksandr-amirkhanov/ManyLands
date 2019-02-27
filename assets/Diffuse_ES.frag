varying highp vec3 vert;
varying highp vec3 vertNormal;
varying highp vec4 col;

uniform highp vec3 lightPos;

void main()
{
    const highp float color_blending = 0.8;

    highp vec3 L = normalize(lightPos - vert);
    highp float NL = max(dot(normalize(vertNormal), L), 0.0);
    highp vec4 c = clamp(color_blending * col + (1.0 - color_blending) * col * NL,
        0.0, 1.0);
    gl_FragColor = c;
}
