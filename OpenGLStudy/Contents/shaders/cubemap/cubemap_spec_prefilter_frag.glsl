#version 420
out vec4 outColor;

in vec3 TexCoords;

uniform samplerCube cubemapTex;

void main()
{    
   vec3 envColor = texture(cubemapTex, TexCoords).rgb;
    envColor = pow(envColor, vec3(1.0/2.2)); 
    outColor = vec4(envColor, 1.0);
}