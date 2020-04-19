#version 420
layout (location = 0) in vec3 pos;

out vec3 TexCoords;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
	mat4 ProjectionMatrix;
	mat4 InvProj;
	mat4 ViewMatrix;
	mat4 InvView;
};

layout(std140, binding = 1) uniform uniformBuffer_drawcall
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};

void main()
{
    TexCoords = pos;
	vec4 _pos = PVMatrix * modelMatrix * vec4(pos, 1.0);
    gl_Position = _pos;
}