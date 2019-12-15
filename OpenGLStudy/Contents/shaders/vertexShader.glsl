#version 330
layout (location = 0) in vec3 pos;

uniform mat4 modelMatrix;


void main(){

	gl_Position = modelMatrix * vec4( 0.4 * pos.x, 0.4 * pos.y, pos.z, 1.0);
}