#version 420
out vec4 color;
uniform vec3 arrowColor;
void main(){

    color = vec4(arrowColor, 1.0);

}