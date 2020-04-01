#version 420
layout (location = 0) out vec3 gAlbedoMetallic;
layout (location = 1) out vec3 gNormalRoughness;

in vec2 texCood0;
in mat3 TBN;

uniform sampler2D AlbedoMap; // 0
uniform sampler2D MetallicMap; // 1
uniform sampler2D RoughnessMap; // 2
uniform sampler2D NormalMap; // 3


void main()
{
	gAlbedoMetallic.rgb = texture(AlbedoMap, TexCoords).rgb;
	gAlbedoMetallic.a = texture(MetallicMap, TexCoords).r;

	vec3 normal = texture(NormalMap, texCood0).rgb;
	normal = normalize(normal * 2.0f - 1.0f);   
	normal = TBN * normal; 
	vec3 normalized_normal = normalize(normal); // vec3 is in (-1,1)
	normalized_normal = (normalized_normal + vec3(1,1,1)) / 2.0; // normal is in (0,1)
	gNormalRoughness.rgb = normalized_normal;
	gNormalRoughness.a = float(texture(RoughnessMap, texCood0).r);

}