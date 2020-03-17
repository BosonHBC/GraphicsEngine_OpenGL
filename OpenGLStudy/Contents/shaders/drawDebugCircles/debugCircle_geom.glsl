#version 420
layout(points) in;
layout(line_strip, max_vertices = 64)out;

const int MAX_VERTICES = 16;
const float PI = 3.14159265359;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	// PVMatrix stands for projection * view matrix
	mat4 PVMatrix;
	vec3 ViewPosition;
};
uniform float radius;

void main()
{
	vec4 pointWorldPos = gl_in[0].gl_Position;

	for(int i = 0; i < MAX_VERTICES + 1; i++)
  	{
		float theta = 2 * PI * i/float(MAX_VERTICES);
        gl_Position = PVMatrix * vec4(pointWorldPos.x + radius*cos(theta), pointWorldPos.y + radius *sin(theta), pointWorldPos.z, pointWorldPos.w);   //circle parametric equation
 		
		EmitVertex();      
    }
	EndPrimitive();
	for(int i = 0; i < MAX_VERTICES + 1; i++)
  	{
		float theta = 2 * PI * i/float(MAX_VERTICES);
        gl_Position = PVMatrix * vec4(pointWorldPos.x , pointWorldPos.y + radius *sin(theta), pointWorldPos.z + radius*cos(theta), pointWorldPos.w);   //circle parametric equation
 		
		EmitVertex();      
    }
	EndPrimitive();
	for(int i = 0; i < MAX_VERTICES + 1; i++)
  	{
		float theta = 2 * PI * i/float(MAX_VERTICES);
        gl_Position = PVMatrix * vec4(pointWorldPos.x + radius *sin(theta), pointWorldPos.y , pointWorldPos.z + radius*cos(theta), pointWorldPos.w);   //circle parametric equation
 		
		EmitVertex();      
    }
	EndPrimitive();
}