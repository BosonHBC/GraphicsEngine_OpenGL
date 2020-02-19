#pragma once
#include "glm/glm.hpp"
#include "Graphics/Color/Color.h"
// representation of vec3 in uniform buffer only

namespace Graphics {
	namespace UniformBufferFormats {
		struct vec3
		{
			float x, y, z;
			vec3(const glm::vec3& v3) : x(v3.x), y(v3.y), z(v3.z) {}
		};
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

		struct sBlinnPhongMaterial
		{
			glm::vec3 kd;
			// this padding is required when it is a vec3 at the beginning of the struct
			// but why??????
			float padding;
			glm::vec3 ks;
			float shininess;
			sBlinnPhongMaterial() { }

		};


	}
}