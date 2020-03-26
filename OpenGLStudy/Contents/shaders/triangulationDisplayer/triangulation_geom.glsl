#version 420 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

const float MAGNITUDE = 1;

void GenerateLine(int id1, int id2)
{
    gl_Position = gl_in[id1].gl_Position + vec4(0,0,-gl_in[id1].gl_Position.z, 0.0) * MAGNITUDE;
    EmitVertex();
    gl_Position = gl_in[id2].gl_Position + vec4(0,0,-gl_in[id2].gl_Position.z, 0.0) * MAGNITUDE;
    EmitVertex();
    EndPrimitive();

    
}

void main()
{
    GenerateLine(0, 1); // A-B
    GenerateLine(1, 2); // B-C
    GenerateLine(2, 0); // C-A
} 