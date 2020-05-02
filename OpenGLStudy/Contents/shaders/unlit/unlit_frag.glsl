#version 420
out vec4 color;
uniform vec3 unlitColor;
void main(){

    color = vec4(unlitColor, 1.0);

}