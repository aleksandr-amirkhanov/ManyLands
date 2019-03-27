varying mediump vec3 vert;
varying mediump vec3 vertNormal;
varying mediump vec4 col;
varying mediump vec4 viewSpace;

uniform mediump vec3 lightPos;
uniform mediump vec2 fogRange;

const mediump vec3 specColor = 0.3*vec3(1.0, 1.0, 1.0);
const mediump float ambientCoef = 0.3;
const mediump float shininess = 4.0;

void main()
{
    mediump vec3 normal = normalize(vertNormal);
    mediump vec3 lightDir = normalize(lightPos - vert);
    
    mediump float lambertian = max(dot(lightDir, normal), 0.0);
    mediump float specular = 0.0;

    if(lambertian > 0.0)
    {
        mediump vec3 reflectDir = reflect(-lightDir, normal);        
        mediump vec3 viewDir = lightDir;

        mediump float specAngle = max(dot(reflectDir, viewDir), 0.0);
        specular = pow(specAngle, shininess);
    }

    mediump vec4 fragColor = vec4((ambientCoef + lambertian) * col.xyz + specular * specColor, col.w);

    // Fog
    mediump float dist      = abs(viewSpace.z);
    mediump float fogFactor = (fogRange[1] - dist)/(fogRange[1] - fogRange[0]);
    fogFactor       = clamp(fogFactor, 0.0, 1.0);

    if(fogRange[0] < fogRange[1])
        gl_FragColor = vec4(mix(vec3(1), fragColor.xyz, fogFactor), fragColor.w);
    else
        gl_FragColor = fragColor;
}
