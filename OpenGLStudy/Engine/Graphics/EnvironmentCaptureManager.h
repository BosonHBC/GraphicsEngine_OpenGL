#pragma once
#include "Graphics/EnvironmentProbes/EnvProbe.h"
#include "Math/Shape/Sphere.h"
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

			cSphere BV;								// The bounding volume of this probe, should not change after initialization; 
			cSphere InnerBV;						// If a sphere is inside the inner sphere, this weight should be 1
			float Influence;								// 1 at the inner boundary; 0 at the outer boundary; > 1 inside the inner boundary. < 0, outside the outer boundary
			float Resolution;							// Resolution of Environment cubemap(each size)

			sCaptureProbes() {}
			sCaptureProbes(const cSphere& i_sphere, const cSphere& i_innerSphere, float i_influence, float i_resolution) : BV(i_sphere), InnerBV(i_innerSphere), Influence(i_influence), Resolution(i_resolution) { }
			sCaptureProbes(const sCaptureProbes& i_other) :
				EnvironmentProbe(i_other.EnvironmentProbe), IrradianceProbe(i_other.IrradianceProbe), PrefilterProbe(i_other.PrefilterProbe),
				BV(i_other.BV), InnerBV(i_other.InnerBV), Influence(i_other.Influence), Resolution(i_other.Resolution) {}
			sCaptureProbes& operator = (const sCaptureProbes& i_other) {
				EnvironmentProbe = i_other.EnvironmentProbe;
				IrradianceProbe = i_other.IrradianceProbe;
				PrefilterProbe = i_other.PrefilterProbe;
				BV = i_other.BV;
				InnerBV = i_other.InnerBV;
				Influence = i_other.Influence;
				Resolution = i_other.Resolution;
				return *this;
			}
			bool operator < (const sCaptureProbes& i_other) { return (*this).Influence < i_other.Influence; }
			void CleanUp() {
				EnvironmentProbe.CleanUp();
				IrradianceProbe.CleanUp();
				PrefilterProbe.CleanUp();
			}

			// 1 at the inner boundary; 0 at the outer boundary; > 1 inside the inner boundary. < 0, outside the outer boundary
			void CalcInfluenceWeight(const glm::vec3& i_POI)
			{
				float dist2center = glm::distance(i_POI, BV.c());
				Influence = 1 - (dist2center - InnerBV.r()) / (BV.r() - InnerBV.r());
			}
		};

		// Initialization and clean up
		bool Initialize();
		bool CleanUp();
		bool AddCaptureProbes(const cSphere& i_outerSphere, float i_innerRadius, GLuint i_environmentCubemapSize);

		// Pre-render frame, after adding all capture probes, start to capture
		void CaptureEnvironment(Graphics::sDataRequiredToRenderAFrame* i_renderThreadData);

		// During rendering, update the weights according to the point of interest
		void UpdatePointOfInterest(const glm::vec3& i_position);

		// After adding all capture probes, build the octTree
		void BuildAccelerationStructure();

		const std::vector<sCaptureProbes*>& GetCapturesReferences();
		const sCaptureProbes& GetCaptureProbesAt(int i_idx);
		GLuint GetReadyCapturesCount();
		const GLuint MaximumCubemapMixingCount();
	}
}