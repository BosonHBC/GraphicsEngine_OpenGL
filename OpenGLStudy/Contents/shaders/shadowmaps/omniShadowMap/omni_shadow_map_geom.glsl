#version 420
layout (triangles) in;
layout (triangles_strip, max_vertices = 18) out;

uniform mat4 lightMatrices[6];

out vec4 FragPos;

void main()
{
	// f stands for face, this outter for loop draws 6 triangles facing to different direction
	for(int f  = 0; f < 6; ++f)
	{
		gl_Layer = f;
		for(int i = 0; i < 3; ++i)
		{
			// gl_in has 3 values because the in is [triangle]
			FragPos = gl_in[i].gl_Position; // Set FragPos in world space
			// now gl_Position is in clip space according to the lightMatrices
			gl_Position = lightMatrices[f] * FragPos;
			EmitVertex();
		}
		EndPrimitive();
	}
}