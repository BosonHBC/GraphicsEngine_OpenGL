#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texcood;

out vec2 TexCoords;
layout(std140, binding = 1) uniform uniformBuffer_drawcall
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};
void main()
{
    TexCoords = texcood;
	gl_Position = modelMatrix * vec4(aPos, 1.0);
}