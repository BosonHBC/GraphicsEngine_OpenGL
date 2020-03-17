#version 420
layout (location = 0) in vec3 pos;

out vec3 TexCoords;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
    // The position of the viewpoint
    vec3 ViewPosition;
};

void main()
{
    TexCoords = pos;
	gl_Position = PVMatrix * vec4(pos, 1.0);
}
