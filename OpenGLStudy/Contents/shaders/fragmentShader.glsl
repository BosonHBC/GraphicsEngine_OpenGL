#version 330

// this sampler is connected to the texture unit, and binding with our texture
// this uniform is default 0, if we need more texture unit, we need to bind manually
uniform sampler2D diffuseTex;

in vec2 texCood0;
in vec3 Normal;
in vec3 fragPos;

// the color of the pixel
out vec4 color;

const int MAX_COUNT_PER_LIGHT = 3;

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

struct Material{
	float shininess;
};

uniform int pointLightCount;

uniform AmbientLight ambientLight;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_COUNT_PER_LIGHT];

uniform Material material;

uniform vec3 camPos;

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

vec4 CalcDirectionLight(){
	return IlluminateByDirection(directionalLight.base, directionalLight.direction);
}

vec4 CalcPointLight(){
	vec4 outColor = vec4(0,0,0,0);

	for(int i = 0; i < pointLightCount; ++i){
		vec3 dir = fragPos - pointLights[i].position;
		float dist = length(dir);
		dir = normalize(dir);

		vec4 color = IlluminateByDirection(pointLights[i].base, dir);
		float attenuationFactor = pointLights[i].quadratic * dist * dist + 
													pointLights[i].linear * dist + 
													pointLights[i].constant;

		outColor += (color / attenuationFactor);
	}
	return outColor;
}

void main(){
	
	vec4 ambientLightColor = vec4(ambientLight.base.color, 1.0f) * ambientLight.base.diffuseIntensity;
	vec4 directionLightColor = CalcDirectionLight();
	vec4 pointLightColor = CalcPointLight();

	color = texture(diffuseTex, texCood0) * ( ambientLightColor + directionLightColor + pointLightColor);

}