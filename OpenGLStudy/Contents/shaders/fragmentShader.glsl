#version 330

// this sampler is connected to the texture unit, and binding with our texture
// this uniform is default 0, if we need more texture unit, we need to bind manually
uniform sampler2D theTexture;

in vec2 texCood0;
in vec3 Normal;

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

uniform AmbientLight ambientLight;
uniform DirectionalLight directionalLight;

void main(){
	
	vec4 ambientColor = vec4(ambientLight.color, 1.0f) * ambientLight.intensity;

	float vN_Dot_vL = max( dot( Normal , directionalLight.direction), 0.0f );
	vec4 directionalLightColor = vec4(directionalLight.color , 1.0f) * directionalLight.intensity * vN_Dot_vL;

	color = texture(theTexture, texCood0) * ( ambientColor + directionalLightColor);

}