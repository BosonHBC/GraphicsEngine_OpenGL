#pragma once
#include "Graphics/EnvironmentProbes/EnvProbe.h"
namespace Graphics
{
	// Forward declaration
	struct sDataRequiredToRenderAFrame;
	namespace EnvironmentCaptureManager
	{
		struct sCaptureProbes
		{
			cEnvProbe EnvironmentProbe;	// Environment probe, it capture the very detail version of the environment, by rendering the whole scene(actual geometries) 6 times. 
			cEnvProbe IrradianceProbe;		// Storing the irradiance map for ambient lighting, by rendering a cube map to a cube and convolute it with special fragment shader
			cEnvProbe PrefilterProbe;			// pre-filtering cube map for environment reflection
			
			glm::vec3 Position;						// The center of the probe, this should not change after initialization
			float Radius;								// The radius of the probe, if a sphere is inside another one, the smaller sphere should have higher influence
			float Influence;								// The influence of this probe, clamp to [0,1]; dynamically changes depends on the POI.
			float Resolution;						// Resolution of Environment cubemap(each size)

			sCaptureProbes() {}
			sCaptureProbes(const glm::vec3&i_position, float i_radius, float i_resolution) : Position(i_position), Radius(i_radius), Resolution(i_resolution) {}
			sCaptureProbes(const sCaptureProbes& i_other) :
				EnvironmentProbe(i_other.EnvironmentProbe), IrradianceProbe(i_other.IrradianceProbe), PrefilterProbe(i_other.PrefilterProbe),
				Position(i_other.Position), Radius(i_other.Radius), Influence(i_other.Influence), Resolution(i_other.Resolution) {}
			void CleanUp() {
				EnvironmentProbe.CleanUp();
				IrradianceProbe.CleanUp();
				PrefilterProbe.CleanUp();
			}
		};

		// Initialization and clean up
		bool Initialize();
		bool CleanUp();
		bool AddCaptureProbes(const glm::vec3& i_position, GLfloat i_radius, GLuint i_environmentCubemapSize);
		
		// Pre-render frame, after adding all capture probes, start to capture
		void CaptureEnvironment(Graphics::sDataRequiredToRenderAFrame* i_renderThreadData);

		// During rendering, update the weights according to the point of interest
		void UpdatePointOfInterest(const glm::vec3& i_position);

		const sCaptureProbes& GetCaptureProbesAt(int i_idx);
	}
}