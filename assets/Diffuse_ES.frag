varying highp vec3 vert;
varying highp vec3 vertNormal;
varying highp vec4 col;
varying highp vec4 viewSpace;

uniform highp vec3 lightPos;
uniform highp vec2 fogRange;

void main()
{
    const highp float color_blending = 0.8;

    highp vec3 L = normalize(lightPos - vert);
    highp float NL = max(dot(normalize(vertNormal), L), 0.0);
    highp vec4 c = clamp(color_blending * col + (1.0 - color_blending) * col * NL,
        0.0, 1.0);

    highp float dist = abs(viewSpace.z);
    highp float fogFactor = (fogRange[1] - dist)/(fogRange[1] - fogRange[0]);
    fogFactor             = clamp(fogFactor, 0.0, 1.0);

    if(fogRange[0] < fogRange[1])
        gl_FragColor = vec4(mix(vec3(1), c.xyz, fogFactor), c.w);
    else
        gl_FragColor = c;
}
