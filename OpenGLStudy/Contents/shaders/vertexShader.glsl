#version 330
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texcood;


uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec2 texCood0;

void main()
{
	gl_Position =  projectionMatrix * viewMatrix * modelMatrix *vec4(pos.x, pos.y, pos.z, 1.0);
	texCood0 = texcood;
}