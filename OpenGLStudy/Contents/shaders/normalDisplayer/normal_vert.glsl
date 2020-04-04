#version 420
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texcood;
layout (location = 2) in vec3 normal;

out VS_OUT
{
	vec3 Normal;
} vs_out;

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
	gl_Position = PVMatrix * modelMatrix * vec4(pos, 1.0);

	vs_out.Normal = normalize(mat3(normalMatrix) * normal);
}