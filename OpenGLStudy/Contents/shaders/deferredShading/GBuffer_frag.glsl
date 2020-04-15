#version 420
layout (location = 0) out vec4 gAlbedoMetallic;
layout (location = 1) out vec4 gNormalRoughness;
layout (location = 2) out vec4 gIOR;

in vec2 texCood0;
in mat3 TBN;

uniform sampler2D AlbedoMap; 	// 0
uniform sampler2D MetallicMap; 	// 1
uniform sampler2D RoughnessMap; // 2
uniform sampler2D NormalMap; 	// 3
uniform sampler2D AOMap;		// 5

layout(std140, binding = 5) uniform g_uniformBuffer_pbrMRModel
{
	vec3 diffuseIntensity;
	float roughnessIntensity;
	vec3 ior;
	float metalnessIntensity;
};

void main()
{
	gAlbedoMetallic.rgb = texture(AlbedoMap, texCood0).rgb * diffuseIntensity;
	gAlbedoMetallic.a = texture(MetallicMap, texCood0).r * metalnessIntensity;

	vec3 normal = texture(NormalMap, texCood0).rgb;
	normal = normalize(normal * 2.0f - 1.0f);  // vec3 is in (-1,1)
	normal = TBN * normal; 
	vec3 normalized_normal = normalize(normal); 
	gNormalRoughness.rgb = normalized_normal.rgb;
	gNormalRoughness.a = float(texture(RoughnessMap, texCood0).r) * roughnessIntensity;
	
	float textureAO = texture(AOMap, texCood0).r;
	gIOR = vec4(ior.rgb, textureAO);
}