#pragma once
#include "GL/glew.h"
#include "glfw/glfw3.h"
#include "Graphics/Effect/Shader/GLShader.h"

namespace ComputeShaderTest
{
#define NUM_PARTICLES 1024*1024 // total number of particles to move
#define WORK_GROUP_SIZE 128 // # work-items per work-group


	struct sPos
	{
		float x, y, z, w; // positions
	};
	struct sVel
	{
		float vx, vy, vz, vw; // velocities
	};
	struct sColor
	{
		float r, g, b, a; // colors
	};
	// need to do the following for both position, velocity, and colors of the particles:

	bool Init();
	void cleanUp();
	void RenderParticle();
}
