#version 420
out vec4 color;
void main(){
    color = gl_FragCoord.z * vec4(1,1,1,1);

}