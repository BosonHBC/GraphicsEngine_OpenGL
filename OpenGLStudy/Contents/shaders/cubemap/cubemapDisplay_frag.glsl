#version 420
out vec4 outColor;

in vec3 TexCoords;

uniform samplerCube cubemapTex;

void main()
{    
    vec3 dir = normalize(TexCoords);
    float envColor = texture(cubemapTex, dir).r;

    outColor = vec4(vec3(envColor), 1.0);
}