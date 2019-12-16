#version 330

// vertex color coming from vertex shader
in vec4 vertexColor;

// the color of the pixel
out vec4 color;

void main(){

	color = vertexColor;

}