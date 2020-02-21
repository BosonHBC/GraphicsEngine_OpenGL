#version 420
layout (location = 0) in vec3 pos;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	// PVMatrix stands for projection * view matrix
	mat4 PVMatrix;

	vec3 ViewPosition;
	float padding;
};
layout(std140, binding = 1) uniform uniformBuffer_drawcall
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};

void main()
{
	gl_Position = PVMatrix * modelMatrix * vec4(pos, 1.0);
}