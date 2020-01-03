#version 330

// this sampler is connected to the texture unit, and binding with our texture
// this uniform is default 0, if we need more texture unit, we need to bind manually
uniform sampler2D diffuseTex;

in vec2 texCood0;
in vec3 Normal;
in vec3 fragPos;

// the color of the pixel
out vec4 color;

// Lighting, no interpoloation
struct AmbientLight{
	vec3 color;
	float intensity;
};
struct DirectionalLight{
	vec3 color;
	float intensity;
	vec3 direction;
};

struct Material{
	float shininess;
};

uniform AmbientLight ambientLight;
uniform DirectionalLight directionalLight;
uniform Material material;

uniform vec3 camPos;

void main(){
	
	vec4 ambientColor = vec4(ambientLight.color, 1.0f) * ambientLight.intensity;

	float vN_Dot_vL = max( dot( Normal , directionalLight.direction), 0.0f );
	vec4 directionalLightColor = vec4(directionalLight.color , 1.0f) * directionalLight.intensity * vN_Dot_vL;

	if(vN_Dot_vL > 0.0f)
	{
		vec3 fragToEye = normalize(camPos - fragPos);
		vec3 reflectedVertex = normalize( reflect( directionalLight.direction, normalize(Normal) ) );
		
		float specularFactor = dot(fragToEye, reflectedVertex);

		if(specularFactor > 0){
			specularFactor = pow(specularFactor, material.shininess);
			directionalLightColor += vec4(directionalLight.color, 1.f) * directionalLight.intensity  * specularFactor;
		}
	}

	color = texture(diffuseTex, texCood0) * ( ambientColor + directionalLightColor);

}