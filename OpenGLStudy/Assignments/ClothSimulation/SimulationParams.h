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

namespace ClothSim
{
	// rendering data
// --------------------------------------------
// Vertices per cloth edge
#define CLOTH_RESOLUTION 15
	// cloth length in meters
#define CLOTH_LENGTH 200.0
	const int VC = CLOTH_RESOLUTION * CLOTH_RESOLUTION;

	// Rest length for structural, shear and blend constrains;
	extern const float g_structRestLen;
	extern const float g_shearRestLen;
	extern const float g_blendRestLen;

	// stiff and damping for spring particles
#define STRUCT_STIFF 50
#define STRUCT_DAMP	-0.5f
#define SHEAR_STIFF 30
#define SHEAR_DAMP	-0.5f
#define BEND_STIFF 30
#define BEND_DAMP	-0.5f

#define MASS 1.f // 1 kg
#define GRAVITY glm::vec3(0, -9.8f * 3.f , 0)
#define GRAVITY_DAMPING -0.30f
#define DEFAULT_DAMPING - 0.20f
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

	void InitializeNeghbors();
// Using discrete time step
	void UpdateSprings(const float dt);
	void MoveFixedNode(const glm::vec3& i_deltaPosition);
	void CleanUpData();

	float* GetVertexData();
	const std::vector<unsigned int>& GetIndexData();
}

#endif // !SIM_PARAMS_DEFINED

