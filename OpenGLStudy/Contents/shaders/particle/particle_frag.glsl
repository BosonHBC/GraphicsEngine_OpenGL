#version 430

flat in int bNeedRender;
out vec4 color;

void main(){
    if(bNeedRender == 0) discard;
    color = vec4(1,0,0,1);
}