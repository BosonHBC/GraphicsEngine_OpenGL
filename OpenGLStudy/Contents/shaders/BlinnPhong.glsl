#version 420

// this sampler is connected to the texture unit, and binding with our texture
// this uniform is default 0, if we need more texture unit, we need to bind manually
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;

// shadow maps
uniform sampler2D directionalShadowMap;

in vec2 texCood0;
in vec3 Normal;
in vec3 fragPos;

in vec4 DirectionalLightSpacePos;

// the color of the pixel
out vec4 color;

const int MAX_COUNT_PER_LIGHT = 5;

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
	
};
struct PointLight{
	Light base;
	vec3 position;
	float constant;
	float linear;
	float quadratic;
};
struct SpotLight{
	PointLight base;
	vec3 direction;
	float edge;
};

struct Material{
	vec3 kd;
	vec3 ks;
	float shininess;
};

//-------------------------------------------------------------------------
// Uniform Variables
//-------------------------------------------------------------------------

uniform int pointLightCount;
uniform int spotLightCount;

uniform AmbientLight ambientLight;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_COUNT_PER_LIGHT];
uniform SpotLight spotLights[MAX_COUNT_PER_LIGHT];

uniform Material material;

uniform vec3 camPos;

//-------------------------------------------------------------------------
// Fucntions
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Shadow Fucntions
//-------------------------------------------------------------------------
float CalcDirectionalLightShadowMap(vec3 vN)
{
	vec3 projcoords = DirectionalLightSpacePos.xyz / DirectionalLightSpacePos.w;
	projcoords = (projcoords * 0.5) + 0.5;
	// now projcoords is in normalized coordinate, locates in (0,1)

	float current = projcoords.z;

	// Calculate bias
	vec3 lightDir = normalize(directionalLight.direction);
	const float bias = max(0.005 * (1- dot(vN, lightDir)), 0.0005);

	float shadow = 0.0;

	const vec2 texelSize = 1.0 / textureSize(directionalShadowMap, 0);
	// offset the pixel around center pixel, 3x3	
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			// get the depth value of this position in this light's perpective of view
			float pcfDepth = texture(directionalShadowMap, projcoords.xy + vec2(x,y) * texelSize).r;
			// if the current depth that is rendering is larger than the cloest depth,
			// it is in shadow
			shadow += (current - bias > pcfDepth) ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	shadow = current > 1.0 ? 0.0 : shadow;
	return shadow;	
}

//-------------------------------------------------------------------------
// Lighting Fucntions
//-------------------------------------------------------------------------
vec4 IlluminateByDirection_Kd(Light light, vec3 vN, vec3 vL){

	vec4 outColor = vec4(0,0,0,0);

	float vN_Dot_vL = max( dot( vN , vL), 0.0f );

	vec4 diffuseColor = vec4(material.kd, 1.0f) * vN_Dot_vL;
	outColor += vec4(light.color , 1.0f) * diffuseColor;

	return outColor;
}
vec4 IlluminateByDirection_Ks(Light light, vec3 vN, vec3 vL){

	vec4 outColor = vec4(0,0,0,0);

	vec3 vV = normalize(camPos - fragPos);
	vec3 vH = normalize(vV + vL);
	float specularFactor = max(pow(dot(vH, vN),material.shininess),0.0f);
	vec4 specularColor = vec4(material.ks, 1.0f) * specularFactor;
	outColor += vec4(light.color , 1.0f) *specularColor;

	return outColor;
}
//-------------------------------------------------------------------------
// Light calculation
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// DirectionalLight
//-------------------------------------------------------------------------
vec4 CalcDirectionalLight(vec4 diffusTexCol, vec4 specTexCol, vec3 vN){
		vec3 dir = normalize(directionalLight.direction);
		vec4 color_kd = diffusTexCol * IlluminateByDirection_Kd(directionalLight.base, vN, dir);
		vec4 color_ks = specTexCol * IlluminateByDirection_Ks(directionalLight.base, vN, dir);
		return color_kd + color_ks;
}

//-------------------------------------------------------------------------
// PointLight
//-------------------------------------------------------------------------

vec4 CalcPointLight(PointLight pLight,vec4 diffusTexCol, vec4 specTexCol,vec3 vN){
		vec3 dir = pLight.position - fragPos;
		float dist = length(dir);
		dir = normalize(dir);

		vec4 color_kd = diffusTexCol * IlluminateByDirection_Kd(pLight.base, vN, dir);
		vec4 color_ks = specTexCol * IlluminateByDirection_Ks(pLight.base, vN, dir);
		float attenuationFactor = pLight.quadratic * dist * dist + 
													pLight.linear * dist + 
													pLight.constant;

		return ((color_kd + color_ks) / attenuationFactor);
}

vec4 CalcPointLights(vec4 diffusTexCol, vec4 specTexCol,vec3 vN){
	vec4 outColor = vec4(0,0,0,0);

	for(int i = 0; i < pointLightCount; ++i){
		outColor += CalcPointLight(pointLights[i],diffusTexCol, specTexCol,vN);
	}
	return outColor;
}

//-------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------

void main(){
	
	// shared values
	vec3 nomr_normal = normalize(Normal);
	vec4 diffuseTexColor =texture(diffuseTex, texCood0);
	vec4 specularTexColor =texture(specularTex, texCood0);

	// ambient light
	vec4 ambientLightColor = diffuseTexColor * vec4(ambientLight.base.color, 1.0f) * vec4(material.kd, 1.0f);
	
	// point light
	vec4 pointLightColor = CalcPointLights(diffuseTexColor, specularTexColor, nomr_normal);

	// directional light
	float directionalLightShadowFactor = directionalLight.base.enableShadow ? (1.0 - CalcDirectionalLightShadowMap(nomr_normal)): 1.0;
	vec4 directionLightColor = directionalLightShadowFactor * CalcDirectionalLight(diffuseTexColor, specularTexColor, nomr_normal);

	color =  ( ambientLightColor + pointLightColor + directionLightColor);
}