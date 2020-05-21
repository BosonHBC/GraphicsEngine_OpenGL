#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

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

void main()
{
    TexCoords = aTexCoords;
    mat4 VMMatrix = ViewMatrix * modelMatrix ;
    float d = sqrt(pow(VMMatrix[0][0],2) + pow(VMMatrix[0][1],2) + pow(VMMatrix[0][2],2));
    VMMatrix[0][0]= d; VMMatrix[1][0] = 0; VMMatrix[2][0] = 0;
    VMMatrix[0][1]= 0; VMMatrix[1][1] = d; VMMatrix[2][1] = 0;
    VMMatrix[0][2]= 0; VMMatrix[1][2] = 0; VMMatrix[2][2] = d;
    
    gl_Position = ProjectionMatrix * VMMatrix * vec4(aPos, 1.0);
}