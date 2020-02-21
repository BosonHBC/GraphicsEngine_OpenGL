#version 420
layout (location = 0) in vec3 pos;

out vec3 TexCoords;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
    // The position of the viewpoint
    vec3 ViewPosition;
    float padding;
};

void main()
{
    TexCoords = pos;
	vec4 _pos = PVMatrix * vec4(pos, 1.0);
    gl_Position = vec4(_pos.x, _pos.y, _pos.w-0.0001, _pos.w);
}