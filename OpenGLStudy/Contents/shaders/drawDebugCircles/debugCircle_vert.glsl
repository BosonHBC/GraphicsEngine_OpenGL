#version 420
layout (location = 0) in vec3 pos;

layout(std140, binding = 1) uniform uniformBuffer_drawcall
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};
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
	// pass this to world space
	gl_Position = modelMatrix * vec4(pos, 1.0);
}