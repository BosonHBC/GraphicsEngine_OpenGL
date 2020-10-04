#version 420
// Constants
const float PI = 3.14159265359;
const int MAX_COUNT_PER_LIGHT = 5;
const int MAX_COUNT_CUBEMAP_MIXING = 4;
const float MAX_REFLECTION_LOD = 4.0;
const vec2 uvOffsets[16] = 
{
	vec2(0.0, 0.0),
	vec2(0.5, 0.0),

	vec2(0.00, 0.50),
	vec2(0.25, 0.50),
	vec2(0.00, 0.75),
	vec2(0.25, 0.75),
	vec2(0.50, 0.50),
	vec2(0.75, 0.50),

	vec2(0.500, 0.750),
	vec2(0.625, 0.750),
	vec2(0.500, 0.875),
	vec2(0.625, 0.875),
	vec2(0.750, 0.750),
	vec2(0.875, 0.750),
	vec2(0.750, 0.875),
	vec2(0.875, 0.875)
};

// this sampler is connected to the texture unit, and binding with our texture
// this uniform is default 0, if we need more texture unit, we need to bind manually
uniform sampler2D gAlbedoMetallic; // 0
uniform sampler2D gNormalRoughness; // 1
uniform sampler2D gIOR; // 2
uniform sampler2D gDepth; // 3
uniform sampler2D BrdfLUTMap; // 4

uniform samplerCube IrradianceMap[MAX_COUNT_CUBEMAP_MIXING]; // 5 - 8
uniform samplerCube PrefilterMap[MAX_COUNT_CUBEMAP_MIXING]; // 9 - 12

uniform samplerCube pointLightShadowMap[MAX_COUNT_PER_LIGHT]; // 19-> 23
uniform sampler2D directionalShadowMap; // 24
uniform sampler2D gSSAOMap; 			// 25

const vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);


// in vec2 texCood0;
in vec2 TexCoords;

uniform mat4 directionalLightTransform;

layout(std140, binding = 6) uniform g_uniformBuffer_envCaptureWeight
{
	vec4 envCapWeights;
};
// the color of the pixel
out vec4 color;

//-------------------------------------------------------------------------
// Struct definitions
//-------------------------------------------------------------------------

void swap(inout float a, inout float b)
{
	float temp = a;
	a = b;
	b = temp;
}

// define a volume
struct sVolume
{
	vec3 min;
	vec3 max;

	bool isPointInside(vec3 p)
	{
		return p.x >= min.x && p.y >= min.y && p.z >= min.z 
		&& p.x >= max.x && p.y >= max.y && p.z >= max.z;
	}
	
	bool rayVolumeIntersection(vec3 o, vec3 dir /* assume normalized*/, out float tMin, out float tMax)
	{
		// interfect with 6 plane, and get all intersections points
		// only point that satidfied 1. dir * plane.normal < 0; 2. inside the volume is valid
		// there should only be one point is like this
		
		float tmin = (min.x - o.x) / dir.x; 
    	float tmax = (max.x - o.x) / dir.x; 
 
    	if (tmin > tmax) swap(tmin, tmax); 
 
    	float tymin = (min.y - o.y) / dir.y; 
    	float tymax = (max.y - o.y) / dir.y; 
 
    	if (tymin > tymax) swap(tymin, tymax); 
 
    	if ((tmin > tymax) || (tymin > tmax)) 
        	return false; 
 
    	if (tymin > tmin) 
        	tmin = tymin; 
 
    	if (tymax < tmax) 
        	tmax = tymax; 
 
    	float tzmin = (min.z - o.z) / dir.z; 
    	float tzmax = (max.z - o.z) / dir.z; 
 
    	if (tzmin > tzmax) swap(tzmin, tzmax); 
 
    	if ((tmin > tzmax) || (tzmin > tmax)) 
        	return false; 
 
    	if (tzmin > tmin) 
        	tmin = tzmin; 
 
    	if (tzmax < tmax) 
        	tmax = tzmax; 
		
		tMin = tmin;
		tMax = tmax;
		return true; 
	}
};


struct Light{	
	vec3 color;
	int uniqueID;
	bool enableShadow;
	vec2 padding;
};

// Lighting, no interpoloation
struct AmbientLight{
	Light base;
};
struct DirectionalLight{
	Light base;
	vec3 direction;
};
struct PointLight{
	Light base;
	vec3 position;
	float radius;
	int ShadowMapIdx;	
	int ResolutionIdx;		
};
struct SpotLight{
	PointLight base;
	vec3 direction;
	float edge;
};
const int OMNI_SHADOW_MAP_COUNT = 5;
layout(std140, binding = 3) uniform g_uniformBuffer_Lighting
{
	SpotLight g_spotLights[MAX_COUNT_PER_LIGHT]; // 64 * MAX_COUNT_PER_LIGHT = 240 bytes
	PointLight g_pointLights[OMNI_SHADOW_MAP_COUNT * 16]; //  40 * 80 = 3.2 KB
	DirectionalLight g_directionalLight; // 32 bytes
	AmbientLight g_ambientLight; // 16 bytes	
	int g_pointLightCount; // 4 bytes
	int g_spotLightCount; // 4 bytes
}; 
layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
	mat4 ProjectionMatrix;
	mat4 InvProj;
	mat4 ViewMatrix;
	mat4 InvView;
};
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
float computeDistributionGGX(vec3 N, vec3 H, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float NdotH2 = NdotH * NdotH;

    return (alpha2) / (PI * (NdotH2 * (alpha2 - 1.0f) + 1.0f) * (NdotH2 * (alpha2 - 1.0f) + 1.0f));
}
// Geometry function
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
//k is a remapping of α based on whether we're using the geometry function for either direct lighting or IBL lighting
float GeometrySmith(float NdotV,float NdotL, float roughness)
{
    float ggx1 	= GeometrySchlickGGX(NdotV, roughness);
    float ggx2 	= GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
float computeGeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness)
{
    float NdotL2 = NdotL * NdotL;
    float NdotV2 = NdotV * NdotV;
    float kRough2 = roughness * roughness + 0.0001f;

    float ggxL = (2.0f * NdotL) / (NdotL + sqrt(NdotL2 + kRough2 * (1.0f - NdotL2)));
    float ggxV = (2.0f * NdotV) / (NdotV + sqrt(NdotV2 + kRough2 * (1.0f - NdotV2)));

    return ggxL * ggxV;
}
// Fresnel function
vec3 fresnelSchlick(float NdotV, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 
//-------------------------------------------------------------------------
// Shadow Fucntions
//-------------------------------------------------------------------------
float CalcDirectionalLightShadowMap(vec3 vL, vec3 vN, vec4 DirectionalLightSpacePos)
{
	vec3 normalizedDeviceCoordinate = DirectionalLightSpacePos.xyz / DirectionalLightSpacePos.w;
	normalizedDeviceCoordinate = normalizedDeviceCoordinate * 0.5 + 0.5;

	float current = normalizedDeviceCoordinate.z;

	// Calculate bias
	const float bias = max(0.005 * (1- dot(vN, vL)), 0.0005);

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
vec2 sampleCube(const vec3 v, out int faceIndex)
{
	vec3 vAbs = abs(v);
	float ma;
	vec2 uv;
	if(vAbs.z >= vAbs.x && vAbs.z >= vAbs.y)
	{
		faceIndex = v.z < 0.0 ? 5 : 4;
		ma = 0.5 / vAbs.z;
		uv = vec2(v.z < 0.0 ? -v.x : v.x, -v.y);
	}
	else if(vAbs.y >= vAbs.x)
	{
		faceIndex = v.y < 0.0 ? 3 : 2;
		ma = 0.5 / vAbs.y;
		uv = vec2(v.x, v.y < 0.0 ? -v.z : v.z);
	}
	else
	{
		faceIndex = v.x < 0.0 ? 1 : 0;
		ma = 0.5 / vAbs.x;
		uv = vec2(v.x < 0.0 ? v.z : -v.z, -v.y);
	}
	return uv * ma + 0.5;
}


vec2 ChangeUV(vec2 i_uv, int resIdx)
{
	float scale = 0.5;
	if(resIdx > 1 && resIdx <= 7)
		scale = 0.25;
	else if(resIdx > 7)
		scale = 0.125;

	return i_uv * scale + uvOffsets[resIdx];
}
vec3 ChangeDir(PointLight pLight,int faceIdx, vec2 uv)
{
	vec2 newUV = ChangeUV(uv, pLight.ResolutionIdx);
	float r = pLight.radius;
	float l = 2 * r;
	switch(faceIdx)
	{
		case 0: // px
		return vec3(r, r, r) 	+ vec3(0 , -newUV.y * l , -newUV.x * l);
		break;
		
		case 1: // nx
		return vec3(-r, r, -r) + vec3(0 , -newUV.y * l , newUV.x * l);
		break;
		
		case 2: // py
		return vec3(-r, r, -r) 	+ vec3(newUV.x * l, 0, newUV.y * l);
		break;
		
		case 3: // ny
		return vec3(-r, -r, r) 	+ vec3(newUV.x * l, 0, -newUV.y * l);
		break;
		
		case 4: // pz
		return vec3(-r, r, r) 	+ vec3(newUV.x * l, -newUV.y * l, 0);
		break;
		
		case 5: // nz
		return vec3(r, r, -r) 	+ vec3(-newUV.x * l, -newUV.y * l, 0);
		break;
	}
}
float CalcPointLightShadowMap(PointLight pLight, vec3 worldPos, float dist_vV)
{
	vec3 fragToLight = worldPos - pLight.position;
	float fragToLightLength = length(fragToLight);

	// sample the cube map

	float currentDepth = fragToLightLength / pLight.radius;
	float shadow = 0.0;
	float bias = 0.01;
	float samples = 20;
	// length it goes according to the grid sampler, and it determines the blur amount of the shadow
	float diskRadius = (1.0 + (dist_vV / pLight.radius)) / 5.0; 

	for(int i = 0; i < samples; ++i) 
	{
		int faceIdx;
		vec3 biasedFragToLight = fragToLight + gridSamplingDisk[i] * diskRadius;
		vec2 uv = sampleCube(biasedFragToLight,faceIdx);
		vec3 reconstructDir = ChangeDir(pLight, faceIdx, uv);

		float cloestDepth = texture(pointLightShadowMap[pLight.ShadowMapIdx], reconstructDir).r;
		shadow += (currentDepth - bias > cloestDepth) ? 1.0 : 0.0;
	}

	shadow /= float(samples);
	return shadow;
}

//-------------------------------------------------------------------------
// Lighting Fucntions
//-------------------------------------------------------------------------
// This function is not only cook-torrance brdf, it multiplies the radiance of the light and the geometry term to gether so it gets the 
// irrandance of this light
 vec3 CookTorranceBrdf(vec3 radiance, vec3 albedoColor, float metalness, float roughness, vec3 f0 ,vec3 vN, vec3 vH, vec3 vL, vec3 vV)
 {
	float NdotV = max(dot(vN, vV), 0.0001f);
	// geometry component
    float NdotL = clamp(dot(vN, vL), 0.0, 1.0);
	//float NDF = DistributionGGX(vN, vH, roughness);
	float NDF = computeDistributionGGX(vN, vH, roughness);
	//float G = GeometrySmith(NdotV, NdotL, roughness);
	float G = computeGeometryAttenuationGGXSmith(NdotL, NdotV, roughness);
	//vec3 F = fresnelSchlick(max(dot(vH, vV), 0.0), f0);
	vec3 F = fresnelSchlick(NdotV, f0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= (1.0 - metalness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * NdotV * NdotL + 0.0001f;
	// specular component
	vec3 specular = numerator / denominator;
	// diffuse component
	vec3 diffuse = kD * albedoColor / PI;

	return (diffuse + specular) * radiance * NdotL;
 }

//-------------------------------------------------------------------------
// Light calculation
//-------------------------------------------------------------------------
vec3 GetBlendedIrradTexture(vec3 vN)
{
	vec3 outColor = vec3(0,0,0);
	outColor += ((envCapWeights.x > 0.0) ? envCapWeights.x * texture(IrradianceMap[0], vN).rgb : vec3(0.0));
	outColor += ((envCapWeights.y > 0.0) ? envCapWeights.y * texture(IrradianceMap[1], vN).rgb : vec3(0.0));
	outColor += ((envCapWeights.z > 0.0) ? envCapWeights.z * texture(IrradianceMap[2], vN).rgb : vec3(0.0));		
	outColor += ((envCapWeights.w > 0.0) ? envCapWeights.w * texture(IrradianceMap[3], vN).rgb : vec3(0.0));

	//outColor += envCapWeights.x * texture(IrradianceMap[0], vN).rgb;
	//outColor += envCapWeights.y * texture(IrradianceMap[1], vN).rgb;
	//outColor += envCapWeights.z * texture(IrradianceMap[2], vN).rgb;
	//outColor += envCapWeights.w * texture(IrradianceMap[3], vN).rgb;

	return outColor;
}
vec3 GetBlendedPreFilterTexture(vec3 vR, float roughness, vec3 worldPos, vec3 ViewPosition)
{

	sVolume volume;
	volume.min = vec3(-480,0,-325);
	volume.max = vec3(480, 290, 325);
	float tMax = 0, tMin = 0;
	bool bSucceedFindIntersectionPoint = volume.rayVolumeIntersection(worldPos, vR, tMin, tMax);
	vec3 intersectionPoint = worldPos + vR * tMax;

	vec3 sampleDir = bSucceedFindIntersectionPoint ? intersectionPoint - ViewPosition : vR;
	vec3 outColor = vec3(0,0,0);
	float lod = roughness * MAX_REFLECTION_LOD;
	outColor += ((envCapWeights.x > 0.0) ? envCapWeights.x * textureLod(PrefilterMap[0], sampleDir, lod).rgb : vec3(0.0));
	outColor += ((envCapWeights.y > 0.0) ? envCapWeights.y * textureLod(PrefilterMap[1], sampleDir, lod).rgb : vec3(0.0));
	outColor += ((envCapWeights.z > 0.0) ? envCapWeights.z * textureLod(PrefilterMap[2], sampleDir, lod).rgb : vec3(0.0));		
	outColor += ((envCapWeights.w > 0.0) ? envCapWeights.w * textureLod(PrefilterMap[3], sampleDir, lod).rgb : vec3(0.0));
	//outColor += envCapWeights.x * textureLod(PrefilterMap[0], vR, lod).rgb;
	//outColor += envCapWeights.y * textureLod(PrefilterMap[1], vR, lod).rgb;
	//outColor += envCapWeights.z * textureLod(PrefilterMap[2], vR, lod).rgb;
	//outColor += envCapWeights.w * textureLod(PrefilterMap[3], vR, lod).rgb;
	//outColor = max(outColor, vec3(0));
	return outColor;
}
vec3 CalcAmbientLight(AmbientLight aLight, vec3 albedoColor,float metalness, float roughness, vec3 f0 ,vec3 vN,vec3 vV,vec3 vR, vec3 worldPos, vec3 ViewPosition )
{
	float NdotV = max(dot(vN, vV), 0.0001f);
	vec3 kS = fresnelSchlickRoughness(NdotV, f0, roughness); 
	vec3 kD = vec3(1.0) - kS;
	kD *= (1.0 - metalness);
	vec3 irradiance = GetBlendedIrradTexture(vN);
	vec3 diffuse    = irradiance * albedoColor;
	
	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.    
	vec3 prefilteredColor = GetBlendedPreFilterTexture(vR, roughness, worldPos, ViewPosition);
	//vec2 brdfuv = vec2(NdotV, 1 - roughness);
	vec2 brdfuv = vec2(NdotV, roughness);
    vec2 brdf  = texture(BrdfLUTMap, brdfuv).rg;
    vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);
	
	vec3 ambient = (kD * diffuse + specular) * aLight.base.color /** ao // no ao for now*/; 

	return ambient;
}
//-------------------------------------------------------------------------
// DirectionalLight
//-------------------------------------------------------------------------

vec3 CalcDirectionalLight(DirectionalLight dLight, vec4 LightSpacePosition,
vec3 albedoColor, float metalness, float roughness, vec3 f0, vec3 vN, vec3 vV)
{
	vec3 vL = normalize(dLight.direction);
	vec3 vH = normalize(vV + vL);
	vec3 radiance = dLight.base.color;
	
	vec3 cookedIrrandance = CookTorranceBrdf(radiance, albedoColor, metalness, roughness, f0,vN, vH, vL ,vV);
	float shadowFactor = dLight.base.enableShadow ? (1.0 - CalcDirectionalLightShadowMap(vL, vN, LightSpacePosition)): 1.0;
	shadowFactor = max(shadowFactor, 0.0);
	return shadowFactor * cookedIrrandance;
}
//-------------------------------------------------------------------------
// PointLight
//-------------------------------------------------------------------------

vec3 CalcPointLight(PointLight pLight, vec3 worldPos,
vec3 albedoColor, float metalness, float roughness, vec3 f0, vec3 vN, vec3 vV, float viewDistance){
		vec3 vL = pLight.position - worldPos;
		float dist = length(vL);
		if(dist >  pLight.radius)
			return vec3(0,0,0);
		vL = normalize(vL); // normalzied light direction
		vec3 vH = normalize(vV + vL); // halfway vector
		float distRate = dist / pLight.radius;
		float falloff = pow(clamp(1-pow(distRate,4),0.0,1.0),2) / (1 + dist * dist);
		// 10000.f is a intensity compensation of changing falloff method from Exponential Falloff to Inverse Square Falloff, light unit now is lumens
		vec3 radiance = pLight.base.color * 10000.f * falloff;

		vec3 cookedIrrandance = CookTorranceBrdf(radiance, albedoColor, metalness, roughness, f0,vN, vH, vL ,vV);

		// shadow
		float shadowFactor = 1.0;
		if(pLight.base.enableShadow && pLight.ShadowMapIdx < MAX_COUNT_PER_LIGHT){
			shadowFactor = 1.0 - CalcPointLightShadowMap(pLight, worldPos,viewDistance);
			shadowFactor = max(shadowFactor, 0.0);
		}
		return shadowFactor * cookedIrrandance;
}

vec3 CalcPointLights(vec3 worldPos, vec3 albedoColor, float metalness, float roughtness, vec3 f0, vec3 vN, vec3 norm_vV, float viewDistance){
	// reflectance equation
	vec3 Lo = vec3(0,0,0);

	for(int i = 0; i < g_pointLightCount; ++i){
		Lo += CalcPointLight(g_pointLights[i], worldPos, albedoColor, metalness,roughtness, f0, vN, norm_vV, viewDistance);
	}

	return Lo;
}

//-------------------------------------------------------------------------
// SpotLight
//-------------------------------------------------------------------------



//-------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------
vec4 ViewPosFromDepth(vec2 texCoord, float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(texCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = InvProj * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;
    return viewSpacePosition;
}

void main(){
    vec2 texCoord = TexCoords;
    texCoord.y = 1-texCoord.y;

	vec3 ViewPosition = vec3(InvView[3][0], InvView[3][1], InvView[3][2]);
	float depth = texture(gDepth, texCoord).r;
	vec4 worldPos = InvView * ViewPosFromDepth(texCoord, depth);
	// shared values
	vec3 normal = texture(gNormalRoughness, texCoord).rgb;
	vec3 view = ViewPosition - worldPos.xyz;
	float viewDistance = length(view);
	vec3 normalized_view = normalize(view);
	vec3 vR = reflect(-normalized_view, normal);

	// material property
	vec3 albedoColor =  pow(texture(gAlbedoMetallic, texCoord).rgb, vec3(2.2));
	float metalness = texture(gAlbedoMetallic, texCoord).a;
	float roughness = texture(gNormalRoughness, texCoord).a;
	float ssao = texture(gSSAOMap, texCoord).r;
	vec4 ior_ao = texture(gIOR, texCoord);
	vec3 ior = ior_ao.rgb;
	float textureAO = ior_ao.a;
	vec3 F0 = abs ((1.0 - ior) / (1.0 + ior)); //vec3(0.04);
	F0 = F0 * F0;
	F0 = mix(vec3(0.04), albedoColor, vec3(metalness));

	// ambient light
	vec3 ambientLightColor = CalcAmbientLight(g_ambientLight,albedoColor, metalness, roughness, F0,normal,normalized_view, vR, worldPos.xyz, ViewPosition);
	
	// cubemap light
	//vec4 cubemapColor = IlluminateByCubemap(diffuseTexColor,specularTexColor, normalized_normal, normalized_view);

	// point light
	vec3 pointLightColor = CalcPointLights(worldPos.xyz, albedoColor, metalness,roughness,F0, normal, normalized_view, viewDistance);

	// spot light with shadow
	//vec4 spotLightColor = CalcSpotLights(diffuseTexColor, specularTexColor, normalized_normal, normalized_view);

	// directional light
	vec4 dLightLightSpacePos = directionalLightTransform * worldPos;
	vec3 directionLightColor = CalcDirectionalLight(g_directionalLight, dLightLightSpacePos, albedoColor, metalness,roughness,F0, normal, normalized_view);

	vec3 allColor = (ambientLightColor + pointLightColor + directionLightColor) * ssao * textureAO;

	color =  vec4(allColor, 1.0);
}
