#version 420
in vec4 FragPos;

uniform vec3 LightPos;
uniform float farPlane;

void main(){
    // In world space
    float distance = length(FragPos.xyz - LightPos);
    // normalize to [0,1]
    distance = distance / farPlane;
    gl_FragDepth = distance;
}