#version 420
layout(triangles, equal_spacing, cw) in;

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
// attributes of the output CPs
in vec3 ModelPos_ES_in[];
vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}
void main()
{
	//vec3 p1 = mix(ModelPos_ES_in[0], ModelPos_ES_in[1], gl_TessCoord.x);
   // vec3 p2 = mix(ModelPos_ES_in[2], ModelPos_ES_in[3], gl_TessCoord.x);

    //vec3 pos =mix(p1, p2, gl_TessCoord.y);
	vec3 pos = interpolate3D(ModelPos_ES_in[0], ModelPos_ES_in[1], ModelPos_ES_in[2]);
	gl_Position = PVMatrix * modelMatrix * vec4(pos ,1.0);
}