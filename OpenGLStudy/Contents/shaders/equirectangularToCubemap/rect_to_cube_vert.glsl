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

void main()
{
    TexCoords = pos;
	gl_Position = PVMatrix * vec4(pos, 1.0);
}
