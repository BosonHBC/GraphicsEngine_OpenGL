#pragma once
#include "glm/glm.hpp"
namespace Graphics {
	namespace UniformBufferFormats {
		// Frame data should be update every frame
		struct sFrame
		{
			// PVMatrix stands for projection * view matrix
			glm::f32 PVMatrix[16];
			//glm::f32 ProjectionMatrix[16];
		};
		// Frame data should be update every draw call, like for every geometry
		struct sDrawCall
		{
			glm::f32 ModelMatrix[16];
			glm::f32 NormalMatrix[16];
		};

	}
}