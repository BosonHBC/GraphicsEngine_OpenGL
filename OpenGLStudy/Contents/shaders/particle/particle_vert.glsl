#version 430 core
layout (location = 0) in vec4 aPos;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
	mat4 ProjectionMatrix;
	mat4 InvProj;
	mat4 ViewMatrix;
	mat4 InvView;
};

void main()
{
    gl_Position = PVMatrix * aPos;
}