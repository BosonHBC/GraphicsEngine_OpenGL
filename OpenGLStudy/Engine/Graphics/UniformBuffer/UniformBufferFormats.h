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
#define V1Padding float v1Padding = 0.0f
#define V1Padding2 float v1Padding2 = 0.0f
#define V2Padding glm::vec2 v2Padding = glm::vec2(0.0f ,0.0f)
#define V3Padding glm::vec3 v3Padding= glm::vec2(0.0f,0.0f,0.0f)
#endif


		// Frame data should be update every frame
		// --------------------------------------------------------------------------------------------------------------------------------------------
		struct sFrame
		{
			// PVMatrix stands for projection * view matrix
			glm::mat4 PVMatrix = glm::mat4(1.0f);
			glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
			glm::mat4 InvProj = glm::mat4(1.0f);
			glm::mat4 ViewMatrix = glm::mat4(1.0f);
			glm::mat4 InvView = glm::mat4(1.0f);

			sFrame(): PVMatrix(glm::mat4(1.0f)), ProjectionMatrix(glm::mat4(1.0f)),ViewMatrix(glm::mat4(1.0f)), InvView(glm::mat4(1.0f)), InvProj(glm::mat4(1.0f))
			{}

			sFrame(const glm::mat4& i_projectionMatrix, const glm::mat4& i_viewMatrix)
			{
				PVMatrix = i_projectionMatrix * i_viewMatrix;
				ProjectionMatrix = i_projectionMatrix;
				ViewMatrix = i_viewMatrix;
				InvView = glm::inverse(i_viewMatrix);
				InvProj = glm::inverse(i_projectionMatrix);
			}

			glm::vec3 GetViewPosition() const {
				return glm::vec3(InvView[3][0], InvView[3][1], InvView[3][2]);
			}
		};

		// Draw call data should be update every draw call, like for every geometry
		// --------------------------------------------------------------------------------------------------------------------------------------------
		struct sDrawCall
		{
			glm::mat4 ModelMatrix = glm::mat4(1.0f);
			glm::mat4 NormalMatrix = glm::mat4(1.0f);
			sDrawCall() : ModelMatrix(glm::mat4(1.0f)), NormalMatrix(glm::mat4(1.0f)) {}
			sDrawCall(const glm::mat4& i_model, const glm::mat4& i_normal)
			{
				ModelMatrix = i_model; NormalMatrix = i_normal;
			}
		};

		// Material data for blinnPhong material model
		// --------------------------------------------------------------------------------------------------------------------------------------------
		struct sBlinnPhongMaterial
		{
			Color kd = Color(1,1,1);
			V1Padding;
			Color ks = Color(1, 1, 1);
			float shininess = 2.f;
			Color ke = Color(1, 1, 1);
			V1Padding2;

			sBlinnPhongMaterial() { }
			sBlinnPhongMaterial(const Color& i_kd, const Color& i_ks, const Color& i_ke, const float& i_shininess) :
				kd(i_kd), ks(i_ks), ke(i_ke), shininess(i_shininess)
			{ }
		};

		// Material data for PBR metallic roughness model
		// --------------------------------------------------------------------------------------------------------------------------------------------
		struct sPBRMRMaterial
		{
			Color diffuseIntensity = Color(1,1,1);
			float roughnessIntensity = 1.f;
			glm::vec3 ior = glm::vec3(1,1,1);
			float metalnessIntensity = 1;

			sPBRMRMaterial() : diffuseIntensity(Color::White()), roughnessIntensity(1), ior(glm::vec3(1)) {}
			sPBRMRMaterial(const Color& i_diffuseIntensity, const float& i_roughnessIntensity, const glm::vec3& i_ior, const float& i_metalnessIntensity) : diffuseIntensity(i_diffuseIntensity), roughnessIntensity(i_roughnessIntensity), ior(i_ior), metalnessIntensity(i_metalnessIntensity) {}
		};

		// Lighting data
		// --------------------------------------------------------------------------------------------------------------------------------------------
		// This namespace contains helper struct to define a light uniform buffer
		namespace SupportingData {
			// Lighting data
			struct Light16 {
				Color color; // 12 bytes
				bool enableShadow = false; // 4 bytes

				Light16() : color(Color::Black()), enableShadow(false)
				{}
			};

			// Lighting, no interpolation
			struct AmbientLight16 {
				Light16 base;
			};
			struct DirectionalLight32 {
				Light16 base; // 16 bytes
				glm::vec3 direction = glm::vec3(0, 1, 0); // 12 bytes
				V1Padding; // 4 bytes

				DirectionalLight32() : base(Light16()), direction(glm::vec3(0, 1, 0)) {}
			};
			struct PointLight48 {
				Light16 base; // 16 bytes
				glm::vec3 position = glm::vec3(0, 0, 0); // 12 bytes
				float radius = 100.f;			// 4 bytes
				int ShadowMapIdx = 0;	// 4 bytes
				int ResolutionIdx = 0;		// 4 bytes
				V2Padding;						// 8 bytes
				PointLight48() : position(glm::vec3(0, 0, 0)), radius(100.f)
				{}
			};
			struct SpotLight64 {
				PointLight48 base; // 48 bytes
				glm::vec3 direction = glm::vec3(0, 1, 0); // 12 bytes
				float edge = 1.f; // 4 bytes
				SpotLight64() : direction(glm::vec3(0, 1, 0)), edge(1)
				{}
			};
		}

		struct sLighting
		{
			SupportingData::SpotLight64 spotLights[MAX_COUNT_PER_LIGHT]; // 48 * MAX_COUNT_PER_LIGHT = 240 bytes
			SupportingData::PointLight48 pointLights[OMNI_SHADOW_MAP_COUNT * 16]; // 48 * 80 = 3.8 KB
			SupportingData::DirectionalLight32 directionalLight; // 32 bytes
			SupportingData::AmbientLight16 ambientLight; // 16 bytes
			int pointLightCount = 0; // 4 bytes
			int spotLightCount = 0; // 4 bytes
		};

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

		// Environment capture textures weights
		// --------------------------------------------------------------------------------------------------------------------------------------------
		struct sEnvCaptureWeight
		{
			glm::vec4 Weights = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); // Maximum 4 textures

			sEnvCaptureWeight() : Weights(0) {}
			sEnvCaptureWeight(float w1, float w2, float w3, float w4) : Weights(w1, w2, w3, w4) {}
		};

		// Post processing values
		// --------------------------------------------------------------------------------------------------------------------------------------------
		struct sPostProcessing
		{
			float Exposure;
			bool EnablePostProcessing;
			sPostProcessing() : Exposure(1.0f), EnablePostProcessing(true){}
		};
		// SSAO values
		// --------------------------------------------------------------------------------------------------------------------------------------------
#define SSAO_MAX_SAMPLECOUNT 64
		struct sSSAO
		{
			glm::vec4 Samples[SSAO_MAX_SAMPLECOUNT];
			GLuint SampeCount = SSAO_MAX_SAMPLECOUNT;
			float radius = 10.f;
			float power = 1.0f;
			
			sSSAO() {}
		};
	}
}