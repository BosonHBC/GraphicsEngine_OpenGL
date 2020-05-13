#version 420 core

uniform sampler2D sprite;
in vec2 TexCoords;
out vec4 outColor;

void main()
{
    vec4 textureColor = texture(sprite, TexCoords);
    if(textureColor.a < 0.1)
        discard;
    outColor = textureColor;
}