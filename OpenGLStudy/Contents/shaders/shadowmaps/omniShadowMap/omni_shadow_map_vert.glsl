#version 420
layout (location = 0) in vec3 pos;

layout(std140, binding = 1) uniform uniformBuffer_drawcall
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};

void main()
{
	// pass this to world space
	gl_Position = modelMatrix * vec4(pos, 1.0);
}