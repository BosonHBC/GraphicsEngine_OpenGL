#version 420
in vec4 FragPos;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix; // this variable here is useless
	vec3 ViewPosition;
};
uniform float farPlane;

void main(){
    // In world space
    float distance = length(FragPos.xyz - ViewPosition);
    // normalize to [0,1]
    distance = distance / farPlane;
    gl_FragDepth = distance;
}