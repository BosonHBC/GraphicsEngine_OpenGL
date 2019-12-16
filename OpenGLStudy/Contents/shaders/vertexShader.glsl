#version 330
layout (location = 0) in vec3 pos;

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;


out vec4 vertexColor;

void main(){
//
	gl_Position =  projectionMatrix * modelMatrix *vec4(pos.x, pos.y, pos.z, 1.0);
	vertexColor = vec4(clamp(pos, 0.0, 1.0), 1.0);
}