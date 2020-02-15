#version 330

// this sampler is connected to the texture unit, and binding with our texture
// this uniform is default 0, if we need more texture unit, we need to bind manually
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;

in vec2 texCood0;
in vec3 Normal;
in vec3 fragPos;

// the color of the pixel
out vec4 color;

const int MAX_COUNT_PER_LIGHT = 5;

//-------------------------------------------------------------------------
// Struct definitions
//-------------------------------------------------------------------------

struct Light{	
	vec3 color;
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

vec4 IlluminateByDirection_Kd(Light light, vec3 vL){

	vec4 outColor = vec4(0,0,0,0);
	vec3 vN = normalize(Normal);
	float vN_Dot_vL = max( dot( vN , vL), 0.0f );

	vec4 diffuseColor = vec4(material.kd, 1.0f) * vN_Dot_vL;
	outColor += vec4(light.color , 1.0f) * diffuseColor;

	return outColor;
}

vec4 IlluminateByDirection_Ks(Light light, vec3 vL){

	vec4 outColor = vec4(0,0,0,0);
	vec3 vN = normalize(Normal);

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
vec4 CalcDirectionalLight_Kd(){
		vec3 dir = normalize(directionalLight.direction);
		vec4 color = IlluminateByDirection_Kd(directionalLight.base, dir);
		return color;
}

vec4 CalcDirectionalLight_Ks(){
		vec3 dir = normalize(directionalLight.direction);
		vec4 color = IlluminateByDirection_Ks(directionalLight.base, dir);
		return color;
}

//-------------------------------------------------------------------------
// PointLight
//-------------------------------------------------------------------------
vec4 CalcPointLight_Kd(PointLight pLight){
		vec3 dir = pLight.position - fragPos;
		float dist = length(dir);
		dir = normalize(dir);

		vec4 color = IlluminateByDirection_Kd(pLight.base, dir);
		float attenuationFactor = pLight.quadratic * dist * dist + 
													pLight.linear * dist + 
													pLight.constant;

		return (color / attenuationFactor);
}

vec4 CalcPointLights_Kd(){
	vec4 outColor = vec4(0,0,0,0);

	for(int i = 0; i < pointLightCount; ++i){
		outColor += CalcPointLight_Kd(pointLights[i]);
	}
	return outColor;
}
vec4 CalcPointLight_Ks(PointLight pLight){
		vec3 dir = pLight.position - fragPos;
		float dist = length(dir);
		dir = normalize(dir);

		vec4 color = IlluminateByDirection_Ks(pLight.base, dir);
		float attenuationFactor = pLight.quadratic * dist * dist + 
													pLight.linear * dist + 
													pLight.constant;

		return (color / attenuationFactor);
}

vec4 CalcPointLights_Ks(){
	vec4 outColor = vec4(0,0,0,0);

	for(int i = 0; i < pointLightCount; ++i){
		outColor += CalcPointLight_Ks(pointLights[i]);
	}
	return outColor;
}

//-------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------

void main(){
	
	// ambient light
	vec4 ambientLightColor = vec4(ambientLight.base.color, 1.0f) * vec4(material.kd, 1.0f);
	
	// point light
	vec4 pointLightColor_Kd = CalcPointLights_Kd();
	vec4 pointLightColor_Ks = CalcPointLights_Ks();

	// directional light
	vec4 directionLightColor_kd = CalcDirectionalLight_Kd();
	vec4 directionLightColor_ks = CalcDirectionalLight_Ks();


	vec4 diffuseTexColor =texture(diffuseTex, texCood0);
	vec4 specularTexColor =texture(specularTex, texCood0);
	
	color =  diffuseTexColor * ( ambientLightColor + pointLightColor_Kd + directionLightColor_kd) 
			+ specularTexColor * (pointLightColor_Ks + directionLightColor_ks);
	//color = texture(diffuseTex, texCood0) * ( ambientLightColor + pointLightColor_Kd + pointLightColor_Ks);
}