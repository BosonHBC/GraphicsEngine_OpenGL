#version 420
out vec4 outColor;

in vec3 TexCoords;

uniform samplerCube cubemapTex;

void main()
{    
    outColor = texture(cubemapTex, TexCoords);
}