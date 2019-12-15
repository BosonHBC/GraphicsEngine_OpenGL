#version 330
layout (location = 0) in vec3 pos;

void main(){
	gl_Position = vec4( 0.4 * pos.x, 0.4 * pos.y, pos.z, 1.0);
}