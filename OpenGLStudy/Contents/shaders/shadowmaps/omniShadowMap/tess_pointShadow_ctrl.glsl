#version 420
// define the number of CPs in the output patch
layout (vertices = 3) out;

in vec3 fragPos[];
in vec2 texCood0[];
in mat3 TBN[];

// attributes of the output CPs
out vec3 WorldPos_ES_in[];
out vec2 TexCood_ES_in[];
out mat3 TBN_ES_in[];

uniform float tessLevel;

void main()
{
    
    WorldPos_ES_in[gl_InvocationID] = fragPos[gl_InvocationID];
    TexCood_ES_in[gl_InvocationID]  = texCood0[gl_InvocationID];
    TBN_ES_in[gl_InvocationID]      = TBN[gl_InvocationID];       

    gl_TessLevelOuter[0] = tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    //gl_TessLevelOuter[3] = tessLevel;

    gl_TessLevelInner[0] = tessLevel;
    //gl_TessLevelInner[1] = tessLevel;


}