#version 420
layout (location = 0) in vec3 pos;

out vec3 TexCoords;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
};
void main()
{
    TexCoords = pos;
	vec4 pos = PVMatrix * vec4(pos, 1.0);
    gl_Position = pos.xyww;
}