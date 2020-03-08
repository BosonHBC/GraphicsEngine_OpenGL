#version 420
// this sampler is connected to the texture unit, and binding with our texture
// this uniform is default 0, if we need more texture unit, we need to bind manually
uniform sampler2D diffuseTex; // 0
uniform sampler2D specularTex; // 1
uniform sampler2D directionalShadowMap; // 2
uniform samplerCube cubemapTex; // 3
uniform sampler2D reflectionTex; // 4

const int MAX_COUNT_PER_LIGHT = 5;
uniform sampler2D spotlightShadowMap[1]; // 5 -> 10
uniform samplerCube pointLightShadowMap[1]; // 11-> 15

in vec2 texCood0;
in vec3 Normal;
in vec3 fragPos;
in vec4 clipSpaceCoord;

in vec4 DirectionalLightSpacePos;
in vec4 SpotLightSpacePos;
layout(std140, binding = 0) uniform uniformBuffer_frame
{
	// PVMatrix stands for projection * view matrix
	mat4 PVMatrix;
	vec3 ViewPosition;
};
// the color of the pixel
out vec4 color;



layout(std140, binding = 2) uniform g_uniformBuffer_blinnPhongMaterial
{
	vec3 kd;
	vec3 ks;
	float shininess;
	vec3 ke;
};
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
float CalcSpotLightShadowMap(int idx, vec3 vN)
{
	vec3 normalizedDeviceCoordinate = SpotLightSpacePos.xyz / SpotLightSpacePos.w;
	normalizedDeviceCoordinate = normalizedDeviceCoordinate * 0.5 + 0.5;

	float current = normalizedDeviceCoordinate.z;

	// Calculate bias
	vec3 lightDir = normalize(g_spotLights[idx].direction);
	float _bias = 0.01 * (1.0 - dot(vN, lightDir)) * (1-g_spotLights[idx].edge);
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
			shadow += (current - bias/SpotLightSpacePos.w > pcfDepth) ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	shadow = current > 1.0 ? 0.0 : shadow;
	return shadow;	
}

float CalcPointLightShadowMap(int idx, PointLight pLight)
{
	vec3 fragToLight = fragPos - pLight.position;
	// sample the cube map
	float cloest = texture(pointLightShadowMap[idx], fragToLight).r;

	cloest *= pLight.radius;
	float current = length(fragToLight);

	float bias = 0.05;
	// if current is larger than closest, it is in shadow
	float shadow = current - bias > cloest ? 0.0 : 1.0;

	return shadow;
}

//-------------------------------------------------------------------------
// Lighting Fucntions
//-------------------------------------------------------------------------
vec4 IlluminateByDirection_Kd(Light light, vec3 vN, vec3 vL){

	vec4 outColor = vec4(0,0,0,0);

	float vN_Dot_vL = max( dot( vN , vL), 0.0f );

	vec4 diffuseColor = vec4(kd, 1.0f) * vN_Dot_vL;
	outColor += vec4(light.color , 1.0f) * diffuseColor;

	return outColor;
}
vec4 IlluminateByDirection_Ks(Light light, vec3 vN, vec3 vV,vec3 vL){

	vec4 outColor = vec4(0,0,0,0);
	vec3 vH = normalize(vV + vL);
	float specularFactor = max(pow(dot(vH, vN),shininess),0.0f);
	vec4 specularColor = vec4(ks, 1.0f) * specularFactor;
	outColor += vec4(light.color , 1.0f) *specularColor;

	return outColor;
}

vec4 IlluminateByCubemap(vec4 diffuseColor, vec4 specularColor,vec3 vN, vec3 vV)
{
	vec4 outColor = vec4(0,0,0,0);
	vec3 vR = reflect(-vV, vN);
	
	float vN_Dot_vL = max( dot( vN , vR), 0.0f );

	vec4 _diffuse = vec4(kd, 1.0f) * diffuseColor * vN_Dot_vL;
	vec4 _specular = vec4(ks, 1.0f) * specularColor * 1; // * 1 because VH is normal

	outColor = vec4(ke,1.0) *  texture(cubemapTex, vR) * (_diffuse + _specular);
	return outColor;
}

vec4 IlluminateByReflectionTexture()
{
	vec4 outColor = vec4(0,0,0,0);
	vec2 normalizedDeviceCoordinate = vec2((-clipSpaceCoord.x/clipSpaceCoord.w)*0.5 + 0.5,(clipSpaceCoord.y/clipSpaceCoord.w)*0.5 + 0.5 );
	vec2 reflectionCoords = vec2(normalizedDeviceCoordinate.x, -normalizedDeviceCoordinate.y);
	outColor = texture(reflectionTex, normalizedDeviceCoordinate);
	return outColor;
}
//-------------------------------------------------------------------------
// Light calculation
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// DirectionalLight
//-------------------------------------------------------------------------
vec4 CalcDirectionalLight(vec4 diffusTexCol, vec4 specTexCol, vec3 vN, vec3 vV){
		vec3 dir = normalize(g_directionalLight.direction);
		vec4 color_kd = diffusTexCol * IlluminateByDirection_Kd(g_directionalLight.base, vN, dir);
		vec4 color_ks = specTexCol * IlluminateByDirection_Ks(g_directionalLight.base, vN, vV, dir);
		return color_kd + color_ks;
}

//-------------------------------------------------------------------------
// PointLight
//-------------------------------------------------------------------------

vec4 CalcPointLight(int idx, PointLight pLight,vec4 diffusTexCol, vec4 specTexCol,vec3 vN, vec3 vV){
		vec3 dir = pLight.position - fragPos;
		float dist = length(dir);
		dir = normalize(dir);

		// shadow
		float shadowFactor = pLight.base.enableShadow ? (1.0 - CalcPointLightShadowMap(idx, pLight)) : 1.0;

		float distRate = dist / pLight.radius;
		vec4 color_kd = diffusTexCol * IlluminateByDirection_Kd(pLight.base, vN, dir);
		vec4 color_ks = specTexCol * IlluminateByDirection_Ks(pLight.base, vN, dir, vV);
		float attenuationFactor = (pLight.quadratic * distRate * distRate + 
													pLight.linear * distRate + 
													pLight.constant);

		return shadowFactor * ((color_kd + color_ks) / attenuationFactor);
}

vec4 CalcPointLights(vec4 diffusTexCol, vec4 specTexCol,vec3 vN, vec3 vV){
	vec4 outColor = vec4(0,0,0,0);

	for(int i = 0; i < g_pointLightCount; ++i){
		outColor += CalcPointLight(i, g_pointLights[i],diffusTexCol, specTexCol,vN,vV);
	}
	return outColor;
}

//-------------------------------------------------------------------------
// SpotLight
//-------------------------------------------------------------------------

vec4 CalcSpotLight(int idx, SpotLight spLight,vec4 diffusTexCol, vec4 specTexCol,vec3 vN, vec3 vV){
		vec3 dir = spLight.base.position - fragPos;
		vec3 norm_dir = normalize(dir);
		float dist = length(dir);
		float dir_dot_LDir = dot(-spLight.direction, norm_dir);
		if(dir_dot_LDir > spLight.edge)
		{
			float distRate = dist / spLight.base.radius;
			vec4 color_kd = diffusTexCol * IlluminateByDirection_Kd(spLight.base.base, vN, norm_dir);
			vec4 color_ks = specTexCol * IlluminateByDirection_Ks(spLight.base.base, vN, norm_dir, vV);
			float attenuationFactor = (spLight.base.quadratic * distRate * distRate+ 
													spLight.base.linear * distRate + 
													spLight.base.constant) ;

			vec4 outColor = (color_kd + color_ks)  / attenuationFactor;
			float shadowFactor = spLight.base.base.enableShadow ? (1.0 - CalcSpotLightShadowMap(idx, vN)): 1.0;
			outColor *= shadowFactor;
			return outColor * (1.0f - (1-dir_dot_LDir) * (1.0f / (1.0f - spLight.edge)));
		}
		else
		{
			return vec4(0,0,0,0);
		}

}

vec4 CalcSpotLights(vec4 diffusTexCol, vec4 specTexCol,vec3 vN, vec3 vV){
	vec4 outColor = vec4(0,0,0,0);

	for(int i = 0; i < g_spotLightCount; ++i){
		outColor += CalcSpotLight(i, g_spotLights[i],diffusTexCol, specTexCol,vN,vV);
	}
	return outColor;
}

//-------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------

void main(){
	
	// shared values
	vec3 normalized_normal = normalize(Normal);
	vec3 normalized_view = normalize(ViewPosition - fragPos);
	vec4 diffuseTexColor = texture(diffuseTex, texCood0);
	vec4 specularTexColor =texture(specularTex, texCood0);

	// ambient light
	vec4 ambientLightColor = diffuseTexColor * vec4(g_ambientLight.base.color, 1.0f) * vec4(kd, 1.0f);
	
	// cubemap light
	vec4 cubemapColor = IlluminateByCubemap(diffuseTexColor,specularTexColor, normalized_normal, normalized_view);

	// point light
	vec4 pointLightColor = CalcPointLights(diffuseTexColor, specularTexColor, normalized_normal, normalized_view);

	// spot light with shadow
	vec4 spotLightColor = CalcSpotLights(diffuseTexColor, specularTexColor, normalized_normal, normalized_view);

	vec4 reflectionTextureColor = IlluminateByReflectionTexture();
	// directional light
	float directionalLightShadowFactor = g_directionalLight.base.enableShadow ? (1.0 - CalcDirectionalLightShadowMap(normalized_normal)): 1.0;
	vec4 directionLightColor = directionalLightShadowFactor * CalcDirectionalLight(diffuseTexColor, specularTexColor, normalized_normal, normalized_view);

	color =  ( ambientLightColor + cubemapColor + pointLightColor + spotLightColor + directionLightColor + reflectionTextureColor);
}