#version 420
// Constants
const float PI = 3.1415926;
const int MAX_COUNT_PER_LIGHT = 5;

// this sampler is connected to the texture unit, and binding with our texture
// this uniform is default 0, if we need more texture unit, we need to bind manually
uniform sampler2D AlbedoMap; // 0
uniform sampler2D MetallicMap; // 1
uniform sampler2D RoughnessMap; // 2
uniform sampler2D NormalMap; // 3

uniform sampler2D directionalShadowMap; // 4
uniform sampler2D spotlightShadowMap[MAX_COUNT_PER_LIGHT]; // 6 -> 10
uniform samplerCube pointLightShadowMap[MAX_COUNT_PER_LIGHT]; // 11-> 15

const vec3 gridSamplingDisk[20] =vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

in vec2 texCood0;
in vec3 fragPos;
in vec4 clipSpaceCoord;
in mat3 TBN;

in vec4 DirectionalLightSpacePos;
in vec4 SpotLightSpacePos[MAX_COUNT_PER_LIGHT];
layout(std140, binding = 0) uniform uniformBuffer_frame
{
	// PVMatrix stands for projection * view matrix
	mat4 PVMatrix;
	vec3 ViewPosition;
};
layout(std140, binding = 5) uniform g_uniformBuffer_pbrMRModel
{
	vec3 diffuseIntensity;
	float roughnessIntensity;
	vec3 ior;
	float metalnessIntensity;
};
// the color of the pixel
out vec4 color;

//-------------------------------------------------------------------------
// Struct definitions
//-------------------------------------------------------------------------

struct Light{	
	vec3 color;
	bool enableShadow;
};

// Lighting, no interpoloation
struct AmbientLight{
	Light base;
};
struct DirectionalLight{
	Light base;
	vec3 direction;
	// For vec4 alignment
	float v1Padding;
};
struct PointLight{
	Light base;
	vec3 position;
	float constant;
	float linear;
	float quadratic;
	float radius;
};
struct SpotLight{
	PointLight base;
	vec3 direction;
	float edge;
};
layout(std140, binding = 3) uniform g_uniformBuffer_Lighting
{
	int g_pointLightCount; // 4 bytes
	int g_spotLightCount; // 4 bytes
	// For vec4 alignment
	vec2 g_v2Padding;
	AmbientLight g_ambientLight; // 16 bytes
	DirectionalLight g_directionalLight; // 32 bytes
	PointLight g_pointLights[MAX_COUNT_PER_LIGHT]; // 48 * MAX_COUNT_PER_LIGHT = 240 bytes
	SpotLight g_spotLights[MAX_COUNT_PER_LIGHT]; // 64 * MAX_COUNT_PER_LIGHT = 320 bytes
}; // 624 bytes per lighting data

//-------------------------------------------------------------------------
// Fucntions
//-------------------------------------------------------------------------
// Normal distribution function
float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}
// Geometry function
float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
//k is a remapping of α based on whether we're using the geometry function for either direct lighting or IBL lighting
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 	= GeometrySchlickGGX(NdotV, k);
    float ggx2 	= GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}
// Fresnel function
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
//-------------------------------------------------------------------------
// Shadow Fucntions
//-------------------------------------------------------------------------
float CalcDirectionalLightShadowMap(vec3 vN)
{
	vec3 normalizedDeviceCoordinate = DirectionalLightSpacePos.xyz / DirectionalLightSpacePos.w;
	normalizedDeviceCoordinate = normalizedDeviceCoordinate * 0.5 + 0.5;

	float current = normalizedDeviceCoordinate.z;

	// Calculate bias
	vec3 lightDir = normalize(g_directionalLight.direction);
	const float bias = max(0.005 * (1- dot(vN, lightDir)), 0.0005);

	float shadow = 0.0;

	const vec2 texelSize = 1.0 / textureSize(directionalShadowMap, 0);
	// offset the pixel around center pixel, 3x3	
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			// get the depth value of this position in this light's perpective of view
			float pcfDepth = texture(directionalShadowMap, normalizedDeviceCoordinate.xy + vec2(x,y) * texelSize).r;
			// if the current depth that is rendering is larger than the cloest depth,
			// it is in shadow
			shadow += (current - bias > pcfDepth) ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	shadow = current > 1.0 ? 0.0 : shadow;
	return shadow;	
}
float CalcSpotLightShadowMap(int idx, SpotLight spLight, vec3 vN)
{
	vec3 normalizedDeviceCoordinate = SpotLightSpacePos[idx].xyz / SpotLightSpacePos[idx].w;
	normalizedDeviceCoordinate = normalizedDeviceCoordinate * 0.5 + 0.5;

	float current = normalizedDeviceCoordinate.z;

	// Calculate bias
	vec3 lightDir = normalize(spLight.direction);
	float _bias = 0.05 * (1.0 - dot(vN, lightDir)) * (1-spLight.edge);
	const float bias = max(_bias, 0.05);

	float shadow = 0.0;

	const vec2 texelSize = 1.0 / textureSize(spotlightShadowMap[idx], 0);
	// offset the pixel around center pixel, 3x3	
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			// get the depth value of this position in this light's perpective of view
			float pcfDepth = texture(spotlightShadowMap[idx], normalizedDeviceCoordinate.xy + vec2(x,y) * texelSize).r;
			// if the current depth that is rendering is larger than the cloest depth,
			// it is in shadow
			shadow += (current - bias/SpotLightSpacePos[idx].w > pcfDepth) ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	shadow = current > 1.0 ? 0.0 : shadow;
	return shadow;	
}

float CalcPointLightShadowMap(int idx, PointLight pLight, float dist_vV)
{
	vec3 fragToLight = fragPos - pLight.position;
	// sample the cube map
	
	float currentDepth = length(fragToLight) / pLight.radius;
	float shadow = 0.0;
	float bias = 0.05;
	float samples = 20;
	// length it goes according to the grid sampler, and it determines the blur amount of the shadow
	float diskRadius = (1.0 + (dist_vV / pLight.radius)) / 5.0; 

	for(int i = 0; i < samples; ++i) 
	{
		float cloestDepth = texture(pointLightShadowMap[idx], fragToLight + gridSamplingDisk[i] * diskRadius).r;
		shadow += (currentDepth - bias > cloestDepth) ? 1.0 : 0.0;
	}

	shadow /= float(samples);
	return shadow;
}

//-------------------------------------------------------------------------
// Lighting Fucntions
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Light calculation
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// DirectionalLight
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// PointLight
//-------------------------------------------------------------------------

vec3 CalcPointLight(int idx, PointLight pLight,
vec3 albedoColor, float metalness, float roughness, vec3 f0, vec3 vN, vec3 vV, float viewDistance){
		vec3 vL = pLight.position - fragPos;
		float dist = length(vL);
		vL = normalize(vL); // normalzied light direction
		vec3 vH = normalize(vV + vL); // halfway vector
		float distRate = dist / pLight.radius;
		float attenuationFactor = 1.0 / (pLight.quadratic * distRate * distRate + 
													pLight.linear * distRate + 
													pLight.constant);
		vec3 radiance = pLight.base.color * attenuationFactor;

		// cook-torrance brdf
		float NDF = DistributionGGX(vN, vH, roughness);
		float G = GeometrySmith(vN, vV, vL, roughness);
		vec3 F = fresnelSchlick(max(dot(vH, vV), 0.0), f0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= (1.0 - metalness);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(vN, vV), 0.0) * max(dot(vN, vL), 0.0);
		// specular component
		vec3 specular = numerator / max(denominator, 0.001);
		// diffuse component
		vec3 diffuse = kD * albedoColor / PI;
		// geometry component
		float vLDotvN = max(dot(vL, vN), 0.0);

		// shadow
		float shadowFactor = pLight.base.enableShadow ? (1.0 - CalcPointLightShadowMap(idx, pLight, viewDistance)) : 1.0;
		shadowFactor = max(shadowFactor * (.99f -  distRate), 0.0);

		return shadowFactor * (diffuse + specular) * radiance * vLDotvN;
	
}

vec3 CalcPointLights(vec3 albedoColor, float metalness, float roughtness, vec3 f0, vec3 vN, vec3 norm_vV, float viewDistance){
	// reflectance equation
	vec3 Lo = vec3(0,0,0);

	for(int i = 0; i < g_pointLightCount; ++i){
		Lo += CalcPointLight(i, g_pointLights[i], albedoColor, metalness,roughtness, f0, vN, norm_vV, viewDistance);
	}

	return Lo;
}

//-------------------------------------------------------------------------
// SpotLight
//-------------------------------------------------------------------------



//-------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------

void main(){
	
	// shared values
	vec3 normal = texture(NormalMap, texCood0).rgb;
	normal = normalize(normal * 2.0f - 1.0f);   
	normal = TBN * normal; 
	vec3 normalized_normal = normalize(normal);
	vec3 view = ViewPosition - fragPos;
	float viewDistance = length(view);
	vec3 normalized_view = normalize(view);

	// material property
	vec3 albedoColor = texture(AlbedoMap, texCood0).rgb * diffuseIntensity;
	float metalness = texture(MetallicMap, texCood0).r * metalnessIntensity;
	float roughness = texture(RoughnessMap, texCood0).r * min((roughnessIntensity + 0.01f)/2.0f,1);
	vec3 F0 = abs ((1.0 - ior) / (1.0 + ior)); //vec3(0.04);
	//F0 = F0 * F0;
	F0 = mix(F0, albedoColor, vec3(metalness));

	// ambient light
	vec3 ambientLightColor = albedoColor * g_ambientLight.base.color;
	
	// cubemap light
	//vec4 cubemapColor = IlluminateByCubemap(diffuseTexColor,specularTexColor, normalized_normal, normalized_view);

	// point light
	vec3 pointLightColor = CalcPointLights(albedoColor, metalness,roughness,F0, normalized_normal, normalized_view, viewDistance);

	// spot light with shadow
	//vec4 spotLightColor = CalcSpotLights(diffuseTexColor, specularTexColor, normalized_normal, normalized_view);

	// directional light
	//float directionalLightShadowFactor = g_directionalLight.base.enableShadow ? (1.0 - CalcDirectionalLightShadowMap(normalized_normal)): 1.0;
	//vec4 directionLightColor = directionalLightShadowFactor * CalcDirectionalLight(diffuseTexColor, specularTexColor, normalized_normal, normalized_view);

	vec3 allColor = ambientLightColor + pointLightColor;
	// Reinhard operator tone mapping
	//allColor = allColor / (allColor + vec3(1.0));
	// gama correction
	allColor = pow(allColor, vec3(1.0/2.2)); 

	color =  vec4(allColor, 1.0);
}