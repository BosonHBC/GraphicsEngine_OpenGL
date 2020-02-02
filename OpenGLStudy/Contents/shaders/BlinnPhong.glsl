#version 330

// this sampler is connected to the texture unit, and binding with our texture
// this uniform is default 0, if we need more texture unit, we need to bind manually
uniform sampler2D diffuseTex;

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

vec4 IlluminateByDirection(Light light, vec3 vL){

	vec4 outColor = vec4(0,0,0,0);
	vec3 vN = normalize(Normal);
	float vN_Dot_vL = max( dot( vN , vL), 0.0f );

	vec3 vV = normalize(camPos - fragPos);
	vec3 vH = normalize(vV + vL);
	vec4 diffuseColor = vec4(material.kd, 1.0f) * vN_Dot_vL;
	outColor += vec4(light.color , 1.0f) * diffuseColor;

	float specularFactor = max(pow(dot(vH, vN),material.shininess),0.0f);
	vec4 specularColor = vec4(material.ks, 1.0f) * specularFactor;

	outColor += vec4(light.color , 1.0f) *specularColor;

	return outColor;
}


//-------------------------------------------------------------------------
// Light calculation
//-------------------------------------------------------------------------


vec4 CalcPointLight(PointLight pLight){
		vec3 dir = pLight.position - fragPos;
		float dist = length(dir);
		dir = normalize(dir);

		vec4 color = IlluminateByDirection(pLight.base, dir);
		float attenuationFactor = pLight.quadratic * dist * dist + 
													pLight.linear * dist + 
													pLight.constant;

		return (color / attenuationFactor);
}

vec4 CalcPointLights(){
	vec4 outColor = vec4(0,0,0,0);

	for(int i = 0; i < pointLightCount; ++i){
		outColor += CalcPointLight(pointLights[i]);
	}
	return outColor;
}

//-------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------

void main(){
	
	vec4 ambientLightColor = vec4(ambientLight.base.color, 1.0f);
	vec4 pointLightColor = CalcPointLights();


	//texture(diffuseTex, texCood0)
	color = vec4(1,1,1,1) * ( ambientLightColor + pointLightColor);
	//vec3 n = (Normal + vec3(1,1,1))/2.0f;
	//color = vec4(n,1);
}