#pragma once
#include "glm/glm.hpp"
#include "Graphics/Color/Color.h"
#include "glm/gtc/type_ptr.hpp"

namespace Graphics {
	namespace UniformBufferFormats {
		// representation of vec3 in uniform buffer only
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

			sFrame(const glm::mat4& i_projectionMatrix, const glm::mat4& i_viewMatrix) 
			{
				memcpy(PVMatrix, glm::value_ptr(i_projectionMatrix * i_viewMatrix), sizeof(PVMatrix));
			}
		};

		// Frame data should be update every draw call, like for every geometry
		struct sDrawCall
		{
			glm::f32 ModelMatrix[16];
			glm::f32 NormalMatrix[16];

			sDrawCall(const glm::mat4& i_model, const glm::mat4& i_normal)
			{
				memcpy(ModelMatrix, glm::value_ptr(i_model), sizeof(ModelMatrix));
				memcpy(NormalMatrix, glm::value_ptr(i_normal), sizeof(NormalMatrix));
			}
		};

		struct sBlinnPhongMaterial
		{
			Color kd;
			// this padding is required when it is a vec3 at the beginning of the struct
			// but why??????
			float padding;
			Color ks;
			float shininess;
			sBlinnPhongMaterial() { }
			sBlinnPhongMaterial(const Color& i_kd, const Color& i_ks, const float& i_shininess) :
				kd(i_kd), ks(i_ks), shininess(i_shininess), padding(0)
			{ }
		};


	}
}