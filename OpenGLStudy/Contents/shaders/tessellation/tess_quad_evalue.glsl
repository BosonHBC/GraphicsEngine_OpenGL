#version 420
layout(triangles, equal_spacing, ccw) in;

layout(std140, binding = 0) uniform uniformBuffer_frame
{
	// PVMatrix stands for projection * view matrix
	mat4 PVMatrix;
	vec3 ViewPosition;
};
layout(std140, binding = 1) uniform uniformBuffer_drawcall
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};
// attributes of the output CPs
in vec3 WorldPos_ES_in[];
in vec2 TexCood_ES_in[];
in mat3 TBN_ES_in[];

out vec3 fragPos;
out vec2 texCood0;
out mat3 TBN;

uniform sampler2D NormalMap; // 3
uniform sampler2D displacementMap; // 24
uniform float displaceIntensity;
vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}
mat3 interpolateMat3(mat3 v0, mat3 v1, mat3 v2)
{
    vec3 col0 = interpolate3D(v0[0], v1[0], v2[0]);
	vec3 col1 = interpolate3D(v0[1], v1[1], v2[1]);
	vec3 col2 = interpolate3D(v0[2], v1[2], v2[2]);
	return mat3(normalize(col0), normalize(col1), normalize(col2));
}

vec3 interpoalte3D_Quad(vec3 v0, vec3 v1, vec3 v2, vec3 v3)
{
	vec3 p0 = mix(v0, v1, gl_TessCoord.x);
	vec3 p1 = mix(v3, v2, gl_TessCoord.x);
	return mix(p0, p1, gl_TessCoord.y);
}
void main()
{
	
	texCood0 = interpolate2D(TexCood_ES_in[0], TexCood_ES_in[1],TexCood_ES_in[2]);
	TBN = interpolateMat3(TBN_ES_in[0],	TBN_ES_in[1], TBN_ES_in[2]);
	fragPos = interpolate3D(WorldPos_ES_in[0], WorldPos_ES_in[1],WorldPos_ES_in[2]);
	//fragPos = interpoalte3D_Quad(WorldPos_ES_in[0], WorldPos_ES_in[1], WorldPos_ES_in[2], WorldPos_ES_in[3]);
	// calculate displacement
	float displacement = texture(displacementMap, texCood0).r;
	vec3 normal = texture(NormalMap, texCood0).rgb;
	normal = normalize(normal * 2.0f - 1.0f);   
	normal = normalize(TBN * vec3(0,0,1));
	fragPos += normal * displacement * displaceIntensity;
	
	gl_Position = PVMatrix * vec4(fragPos ,1.0);
}