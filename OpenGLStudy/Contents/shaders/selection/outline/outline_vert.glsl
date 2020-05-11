#version 420
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texcood;
layout (location = 2) in vec3 normal;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
	mat4 ProjectionMatrix;
	mat4 InvProj;
	mat4 ViewMatrix;
	mat4 InvView;
};
layout(std140, binding = 1) uniform uniformBuffer_drawcall
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};

uniform float outlineWidth = 0.15;
void main()
{	
	
	vec3 ViewPosition = vec3(InvView[3][0], InvView[3][1], InvView[3][2]);
	vec3 WorldPos = (modelMatrix * vec4(pos, 1.0)).xyz;
	float dist = distance(ViewPosition, WorldPos);
	float width = clamp(outlineWidth * abs(dist) / 200.0, 0.05, 0.2);

	vec3 scaledPos = pos + normalize(normal) * width;
	gl_Position = PVMatrix * modelMatrix * vec4(scaledPos, 1.0);

}