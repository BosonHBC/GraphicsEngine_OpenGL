#version 420
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texcood;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 biTangent;

out vec2 texCood0;
out mat3 TBN;

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

	texCood0 = texcood;

	// Handle scaling in only one axis situation
	vec3 T = normalize(vec3(modelMatrix * vec4(tangent,   0.0)));
   	vec3 B = normalize(vec3(modelMatrix * vec4(biTangent, 0.0)));
   	vec3 N = normalize(mat3(normalMatrix) * normal);
	TBN = mat3(T, B, N);
}