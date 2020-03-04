#pragma once
#include "glm/glm.hpp"
#include "Cores/Core.h"
#include "Graphics/Color/Color.h"
#include "glm/gtc/type_ptr.hpp"

namespace Graphics {
	// Uniform buffer formats should follow vec4 (4*4 bytes) alignment rules
	namespace UniformBufferFormats {
#ifndef _BufferPaddingDefined
#define _BufferPaddingDefined
#define V1Padding float v1Padding = 0
#define V1Padding2 float v1Padding2 = 0
#define V2Padding glm::vec2 v2Padding = glm::vec2(0,0)
#define V3Padding glm::vec3 v3Padding= glm::vec2(0,0,0)
#endif


		// Frame data should be update every frame
		// --------------------------------------------------------------------------------------------------------------------------------------------
		struct sFrame
		{
			// PVMatrix stands for projection * view matrix
			glm::f32 PVMatrix[16];
			glm::vec3 ViewPosition = glm::vec3(0,0,0);

			sFrame() {}

			sFrame(const glm::mat4& i_projectionMatrix, const glm::mat4& i_viewMatrix)
			{
				memcpy(PVMatrix, glm::value_ptr(i_projectionMatrix * i_viewMatrix), sizeof(PVMatrix));
			}
			sFrame(const glm::mat4& i_PVMatrix)
			{
				memcpy(PVMatrix, glm::value_ptr(i_PVMatrix), sizeof(PVMatrix));
			}
		};

		// Draw call data should be update every draw call, like for every geometry
		// --------------------------------------------------------------------------------------------------------------------------------------------
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

		// Material data for blinnPhong material model
		// --------------------------------------------------------------------------------------------------------------------------------------------
		struct sBlinnPhongMaterial
		{
			Color kd;
			V1Padding;
			Color ks;
			float shininess;
			Color ke;
			V1Padding2;

			sBlinnPhongMaterial() { }
			sBlinnPhongMaterial(const Color& i_kd, const Color& i_ks, const Color& i_ke, const float& i_shininess) :
				kd(i_kd), ks(i_ks), ke(i_ke), shininess(i_shininess)
			{ }
		};

		// Lighting data
		// --------------------------------------------------------------------------------------------------------------------------------------------
		// This namespace contains helper struct to define a light uniform buffer
		namespace SupportingData {
			// Lighting data
			struct Light16 {
				Color color; // 12 bytes
				bool enableShadow; // 4 bytes

				Light16() : color(Color::Black()), enableShadow(false)
				{}
			};

			// Lighting, no interpolation
			struct AmbientLight16 {
				Light16 base;
			};
			struct DirectionalLight32 {
				Light16 base; // 16 bytes
				glm::vec3 direction; // 12 bytes
				V1Padding; // 4 bytes

				DirectionalLight32() : direction(glm::vec3(0, 1, 0)) {}
			};
			struct PointLight48 {
				Light16 base; // 16 bytes
				glm::vec3 position; // 12 bytes
				float constant; // 4bytes
				float linear; // 4bytes
				float quadratic; // 4bytes
				V2Padding; // 8bytes

				PointLight48() : position(glm::vec3(0,0,0)), constant(1), linear(0), quadratic(0)
				{}
			};
			struct SpotLight64 {
				PointLight48 base; // 48 bytes
				glm::vec3 direction; // 12 bytes
				float edge; // 4 bytes
				SpotLight64() : direction(glm::vec3(0, 1, 0)), edge(1)
				{}
			};
		}

		struct sLighting
		{
			int pointLightCount; // 4 bytes
			int spotLightCount; // 4 bytes
			V2Padding; // 8 bytes
			SupportingData::AmbientLight16 ambientLight; // 16 bytes
			SupportingData::DirectionalLight32 directionalLight; // 32 bytes
			SupportingData::PointLight48 pointLights[MAX_COUNT_PER_LIGHT]; // 48 * MAX_COUNT_PER_LIGHT = 240 bytes
			SupportingData::SpotLight64 spotLights[MAX_COUNT_PER_LIGHT]; // 64 * MAX_COUNT_PER_LIGHT = 320 bytes
		}; // 624 bytes per lighting data

		// Clipping plane data
		// --------------------------------------------------------------------------------------------------------------------------------------------
		// Allow maximum 4 clip planes in the uniform buffer
		struct sClipPlane
		{
			glm::vec4 Planes[4];
			sClipPlane() {
				Planes[0] = glm::vec4(0, 0, 0, 0); Planes[1] = glm::vec4(0, 0, 0, 0);
				Planes[2] = glm::vec4(0, 0, 0, 0); Planes[3] = glm::vec4(0, 0, 0, 0);
			}
			sClipPlane(const glm::vec4& i_first, const glm::vec4& i_second = glm::vec4(0, 0, 0, 0), const glm::vec4& i_third = glm::vec4(0, 0, 0, 0), const glm::vec4& i_fourth = glm::vec4(0, 0, 0, 0))
			{
				Planes[0] = i_first;
				Planes[1] = i_second;
				Planes[2] = i_third;
				Planes[3] = i_fourth;
			}
		};
	}
}