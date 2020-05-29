#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 otherData;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	mat4 PVMatrix;
	mat4 ProjectionMatrix;
	mat4 InvProj;
	mat4 ViewMatrix;
	mat4 InvView;
};

flat out int bNeedRender;
uniform float g_lifeTime;
void main()
{
	bNeedRender = 1;
	float elpasedTime = otherData.x;
	if(elpasedTime < 0 )
	{
		bNeedRender = 0;
	}
    gl_Position = PVMatrix * aPos;
}