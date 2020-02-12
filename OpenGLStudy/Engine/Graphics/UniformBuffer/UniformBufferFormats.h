#pragma once
#include "glm/glm.hpp"
namespace Graphics {
	namespace UniformBufferFormats {
		// Frame data should be update every frame
		struct sFrame
		{
			glm::f32* ViewMatrix;
			glm::f32* ProjectionMatrix;
			
			sFrame(): ViewMatrix(nullptr), ProjectionMatrix(nullptr)
			{}
			// This size parameter should not be copied to the original data
			const static unsigned int Size = 
				sizeof(glm::f32) * 16 + 
				sizeof(glm::f32) * 16;
		};
		// Frame data should be update every draw call, like for every geometry
		struct sDrawCall
		{
			glm::f32* ModelMatrix;
			glm::f32* NormalMatrix;
		
			sDrawCall() : ModelMatrix(nullptr), NormalMatrix(nullptr)
			{}
			// This size parameter should not be copied to the original data
			const static unsigned int Size =
				sizeof(glm::f32) * 16 +
				sizeof(glm::f32) * 16;
		};
	}
}