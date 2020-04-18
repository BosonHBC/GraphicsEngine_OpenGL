#version 420
in vec4 FragPos;
out vec4 outColor;
layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
	mat4 ProjectionMatrix;
	mat4 InvProj;
	mat4 ViewMatrix;
	mat4 InvView;
};
uniform float farPlane;

void main(){
    vec3 ViewPosition = vec3(InvView[3][0], InvView[3][1], InvView[3][2]);
    // In world space
    float distance = length(FragPos.xyz - ViewPosition);
    // normalize to [0,1]
    distance = distance / farPlane;
    gl_FragDepth = distance;
    outColor = vec4(distance);
}