#version 330

// this sampler is connected to the texture unit, and binding with our texture
// this uniform is default 0, if we need more texture unit, we need to bind manually
uniform sampler2D theTexture;

in vec2 texCood0;

// the color of the pixel
out vec4 color;

// Lighting, no interpoloation
struct DirectionalLight{
	vec3 color;
	float ambientIntensity;
};

uniform DirectionalLight directionalLight;

void main(){
	
	vec4 ambientColor = vec4(directionalLight.color, 1.0f) * directionalLight.ambientIntensity;

	color = texture(theTexture, texCood0) * ambientColor;

}