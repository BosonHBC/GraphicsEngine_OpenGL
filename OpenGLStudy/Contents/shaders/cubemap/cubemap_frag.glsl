#version 420
out vec4 outColor;

in vec3 TexCoords;

uniform samplerCube cubemapTex;

void main()
{    
    vec3 envColor = texture(cubemapTex, normalize(TexCoords)).rgb;

    outColor = vec4(envColor, 1.0);
}