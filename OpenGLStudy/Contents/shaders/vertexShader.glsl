#version 330
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texcood;
layout (location = 2) in vec3 normal;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec2 texCood0;
out vec3 Normal;

void main()
{
	gl_Position =  projectionMatrix * viewMatrix * modelMatrix *vec4(pos.x, pos.y, pos.z, 1.0);
	texCood0 = texcood;

	// Handle scaling in only one axis situation
	Normal = mat3(transpose(inverse(modelMatrix))) * normal;

}