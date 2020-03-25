#version 420
// define the number of CPs in the output patch
layout (vertices = 3) out;

in vec3 ModelPos[];

// attributes of the output CPs
out vec3 ModelPos_ES_in[];

void main()
{
    
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 18.f;
        gl_TessLevelOuter[1] = 18.f;
        gl_TessLevelOuter[2] = 18.f;
        //gl_TessLevelOuter[3] = 5.f;

        gl_TessLevelInner[0] = 16.f;
       // gl_TessLevelInner[1] = 9.f;
    }
    ModelPos_ES_in[gl_InvocationID] = ModelPos[gl_InvocationID];
}