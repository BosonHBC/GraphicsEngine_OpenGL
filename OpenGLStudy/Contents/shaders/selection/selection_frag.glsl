#version 420
out vec4 color;
uniform vec3 selectionColor;
void main(){

    color = vec4(selectionColor, 1.0);

}