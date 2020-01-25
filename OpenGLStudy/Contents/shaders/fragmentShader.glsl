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
	float diffuseIntensity;
	float specularIntensity;
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

vec4 IlluminateByDirection(Light light, vec3 direction){

	float vN_Dot_vL = max( dot( Normal , direction), 0.0f );
	vec4 diffuseColor = vec4(light.color , 1.0f) * light.diffuseIntensity * vN_Dot_vL;

	vec4 specularColor = vec4(0,0,0,0);
	if(vN_Dot_vL > 0.0f)
	{
		vec3 fragToEye = normalize(camPos - fragPos);
		vec3 reflectedVertex = normalize( reflect( direction, normalize(Normal) ) );
		
		float specularFactor = dot(fragToEye, reflectedVertex);

		if(specularFactor > 0.0f){
			specularFactor = pow(specularFactor, material.shininess);
			specularColor = vec4(light.color, 1.f) * light.specularIntensity  * specularFactor;
		}
	}

	return diffuseColor + specularColor;
}


//-------------------------------------------------------------------------
// Light calculation
//-------------------------------------------------------------------------

vec4 CalcDirectionLight(){
	return IlluminateByDirection(directionalLight.base, directionalLight.direction);
}

vec4 CalcPointLight(PointLight pLight){
		vec3 dir = fragPos - pLight.position;
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

vec4 CalcSpotLight(SpotLight sLight){
	vec3 rayDirection = normalize(fragPos - sLight.base.position);
	float slFactor = dot(rayDirection, sLight.direction);

	if(slFactor > sLight.edge){
		vec4 color = CalcPointLight(sLight.base);
		return color * (1.0f - (1.0f - slFactor) * (1.0f / (1.0f - sLight.edge)));
	}
	else return vec4(0,0,0,0);
}

vec4 CalcSpotLights(){
	vec4 outColor = vec4(0,0,0,0);

	for(int i = 0; i < spotLightCount; ++i){
		outColor += CalcSpotLight(spotLights[i]);
	}
	return outColor;
}

//-------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------

void main(){
	
	vec4 ambientLightColor = vec4(ambientLight.base.color, 1.0f) * ambientLight.base.diffuseIntensity;
	vec4 directionLightColor = CalcDirectionLight();
	vec4 pointLightColor = CalcPointLights();
	vec4 spotLightColor = CalcSpotLights();

	color = texture(diffuseTex, texCood0) * ( ambientLightColor + directionLightColor + pointLightColor + spotLightColor);

}