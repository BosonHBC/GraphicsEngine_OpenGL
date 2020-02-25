#version 420
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texcood;
layout (location = 2) in vec3 normal;

out vec2 texCood0;
out vec3 Normal;
out vec3 fragPos;
out vec4 DirectionalLightSpacePos;
out vec4 clipSpaceCoord;

uniform mat4 directionalLightTransform;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	// PVMatrix stands for projection * view matrix
	mat4 PVMatrix;
	vec3 ViewPosition;
};
layout(std140, binding = 1) uniform uniformBuffer_drawcall
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};

layout(std140, binding = 4) uniform uniformBuffer_ClipPlane
{
	vec4 Planes[4];
};

void main()
{
	clipSpaceCoord = PVMatrix * modelMatrix * vec4(pos, 1.0);
	gl_Position = clipSpaceCoord;

	texCood0 = texcood;

	// Handle scaling in only one axis situation
	Normal = mat3(normalMatrix) * normal;

	vec4 worldPos = modelMatrix * vec4(pos.x, pos.y, pos.z, 1.0);
	fragPos = worldPos.xyz;
	
	gl_ClipDistance[0] = dot(worldPos, Planes[0]);

	// Directional light space
	DirectionalLightSpacePos = directionalLightTransform * modelMatrix * vec4(pos, 1.0);
}