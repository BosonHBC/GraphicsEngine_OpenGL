#version 420
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texcood;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 biTangent;

out vec2 texCood0;
out vec3 fragPos;
out vec4 DirectionalLightSpacePos;

out vec4 clipSpaceCoord;
out vec3 Normal;
out mat3 TBN;

uniform mat4 directionalLightTransform;

const int MAX_COUNT_PER_LIGHT = 5;
uniform mat4 spotlightTransform[MAX_COUNT_PER_LIGHT];
out vec4 SpotLightSpacePos[MAX_COUNT_PER_LIGHT];

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
	vec3 T = normalize(vec3(modelMatrix * vec4(tangent,   0.0)));
   	vec3 B = normalize(vec3(modelMatrix * vec4(biTangent, 0.0)));
   	vec3 N = normalize(Normal);
	TBN = mat3(T, B, N);

	/* // alternative way to calculate tbn with extra cost and little bit improvement
		T = normalize(T - dot(T, N) * N);
		// then retrieve perpendicular vector B with the cross product of T and N
		vec3 B = cross(N, T);
	*/

	vec4 worldPos = modelMatrix * vec4(pos.x, pos.y, pos.z, 1.0);
	fragPos = worldPos.xyz;
	
	gl_ClipDistance[0] = dot(worldPos, Planes[0]);

	// Directional light space
	DirectionalLightSpacePos = directionalLightTransform * modelMatrix * vec4(pos, 1.0);

	for(int i =0; i < MAX_COUNT_PER_LIGHT; ++i)
	{
		SpotLightSpacePos[i] = spotlightTransform[i] * modelMatrix * vec4(pos, 1.0);
	}
	
}