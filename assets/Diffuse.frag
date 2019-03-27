#version 150

in highp vec3 vert;
in highp vec3 vertNormal;
in highp vec4 col;
in highp vec4 viewSpace;

out highp vec4 fragColor;

uniform highp vec3 lightPos;
uniform highp vec2 fogRange;

const highp vec3 specColor = vec3(0.3, 0.3, 0.3);
const highp float ambientCoef = 0.3;
const highp float shininess = 4.0;

void main()
{
    highp vec3 normal = normalize(vertNormal);
    highp vec3 lightDir = normalize(lightPos - vert);
    
    float lambertian = max(dot(lightDir, normal), 0.0);
    float specular = 0.0;

    if(lambertian > 0.0)
    {
        highp vec3 reflectDir = reflect(-lightDir, normal);        
        highp vec3 viewDir = lightDir;

        float specAngle = max(dot(reflectDir, viewDir), 0.0);
        specular = pow(specAngle, shininess);
    }

    fragColor = vec4((ambientCoef + lambertian) * col.xyz + specular * specColor, col.w);

    // Fog
    float dist      = abs(viewSpace.z);
    float fogFactor = (fogRange[1] - dist)/(fogRange[1] - fogRange[0]);
    fogFactor       = clamp(fogFactor, 0.0, 1.0);

    if(fogRange[0] < fogRange[1])
        fragColor = vec4(mix(vec3(1), fragColor.xyz, fogFactor), fragColor.w);
}
