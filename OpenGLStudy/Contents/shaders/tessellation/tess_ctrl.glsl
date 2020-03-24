#version 420
// define the number of CPs in the output patch
layout (vertices = 3) out;

in vec3 fragPos[];
in vec2 texCood0[];
in mat3 TBN[];

// attributes of the output CPs
out vec3 WorldPos_ES_in[];
out vec2 TexCoord_ES_in[];
out mat3 TBN_ES_in[];

void main()
{

}