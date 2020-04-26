/*
	This cloth simulation is taking reference from
	Computer Graphics Course Project, OpenGL-simulated Cloth https://www.youtube.com/watch?v=NsjhU…
	Paper: https://github.com/dayeol/clothsimulation/blob/master/CLOTH%20SIMULATION(FINAL).pdf
*/

#pragma once
/* Define units
*	Mass unit: kg;
*	Distance unit: m; (meter)
*	Time unit: s; (second)
*	Acceleration unit: m / s^2
*/

#ifndef SIM_PARAMS_DEFINED
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


#define SIM_PARAMS_DEFINED
class cSphere;
namespace ClothSim
{
	// rendering data
// --------------------------------------------
// Vertices per cloth edge
#define CLOTH_RESOLUTION 100
	// cloth length in meters
#define CLOTH_LENGTH 200.0
	const int VC = CLOTH_RESOLUTION * CLOTH_RESOLUTION;

	// Rest length for structural, shear and blend constrains;
	extern const float g_structRestLen;
	extern const float g_shearRestLen;
	extern const float g_blendRestLen;

	// stiff and damping for spring particles
#define STRUCT_STIFF 80
#define STRUCT_DAMP	 -2.0
#define SHEAR_STIFF 56
#define SHEAR_DAMP -1.0
#define BEND_STIFF 40
#define BEND_DAMP	-0.5
#define STIFF_DIFFERENCE 0.4f

#define MASS 1.f // 1 kg per node
#define GRAVITY glm::vec3(0, -9.8f * 40.f /CLOTH_RESOLUTION , 0)
#define GRAVITY_DAMPING -0.3f

#define FRICTION_COEFFICENT 0.00f
#define FRICTION_DAMPING - 0.50f
#define TOUCH_DIST_THRESHOLD 1.f
#define FRICTION_BIAS 0.5f
	// Clock wise from struct-shear-bend
// --------------------------------------------
#define NO_Neighbor -1
#define Struct_Up 0
#define Struct_Right 1
#define Struct_Down 2
#define Struct_Left 3
#define Shear_0 4
#define Shear_1 5
#define Shear_2 6
#define Shear_3 7
#define Bend_Up 8
#define Bend_Right 9
#define Bend_Down 10
#define Bend_Left 11
	struct sNeighborParticles
	{
		int idx = NO_Neighbor;
		float restLength = 0;
		float stiff = 0;
		float damp = 0;

		void SetIdx(const int constrainIndex, const int i_vertexIndex)
		{
			if (constrainIndex != NO_Neighbor)
			{
				idx = i_vertexIndex;
				int row = idx / CLOTH_RESOLUTION;
				float ratio = static_cast<float>(row) / CLOTH_RESOLUTION;
				if (constrainIndex >= 0 && constrainIndex < 4)
				{
					restLength = g_structRestLen;
					stiff = STRUCT_STIFF;
					damp = STRUCT_DAMP;
				}
				else if (constrainIndex >= 4 && constrainIndex < 8)
				{
					restLength = g_shearRestLen;
					stiff = SHEAR_STIFF;
					damp = SHEAR_DAMP;
				}
				else if (constrainIndex >= 8 && constrainIndex < 12)
				{
					restLength = g_blendRestLen;
					stiff = BEND_STIFF;
					damp = BEND_DAMP;
				}
				stiff += (1 - ratio) * STIFF_DIFFERENCE * stiff;
			}
			else
				assert(false);
		}
	};


	// SprintNode definition
	// --------------------------------------------
	struct sParticle
	{
		glm::vec3 V = glm::vec3(0);// velocity
		glm::vec3 P = glm::vec3(0);// position
		glm::vec3 pP= glm::vec3(0); // previous position
		bool isFixed = false;
		sNeighborParticles neighbor[12];

		sParticle():V(glm::vec3(0)), P(glm::vec3(0)), pP(glm::vec3(0)) {}
		sParticle(glm::vec3 initialP) : V(glm::vec3(0)), P(initialP), pP(initialP) {}
	};

	// Cloth particles
	extern sParticle g_particles[VC];
	extern glm::vec3 g_positionData[VC];
	extern bool g_bEnableClothSim;
	extern bool g_bDrawNodes;
	void InitializeNeghbors();
// Using discrete time step
	void UpdateSprings(const float dt, cSphere* const i_spheres, const int i_numOfSphere);
	void MoveFixedNode(const glm::vec3& i_deltaPosition);
	void ScaleFixedNode(const glm::vec3& i_deltaPosition);
	void CleanUpData();

	float* GetVertexData();
	const std::vector<unsigned int>& GetIndexData();
}

#endif // !SIM_PARAMS_DEFINED

