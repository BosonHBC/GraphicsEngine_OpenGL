#version 420
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texcood;
layout (location = 2) in vec3 normal;

out vec2 texCood0;
out vec3 Normal;
out vec3 fragPos;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 s_viewMatrix;
	mat4 s_projectionMatrix;
};
layout(std140, binding = 1) uniform uniformBuffer_drawcall
{
	mat4 s_ModelMatrix;
	mat4 s_NormalMatrix;
};
void main()
{
	gl_Position =  s_projectionMatrix * s_viewMatrix * s_ModelMatrix *vec4(pos.x, pos.y, pos.z, 1.0);
	texCood0 = texcood;

	// Handle scaling in only one axis situation
	Normal = mat3(s_NormalMatrix) * normal;

	fragPos =  (s_ModelMatrix * vec4(pos.x, pos.y, pos.z, 1.0)).xyz;
}