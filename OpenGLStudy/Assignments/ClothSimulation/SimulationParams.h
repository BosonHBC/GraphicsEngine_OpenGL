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
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define SIM_PARAMS_DEFINED
namespace ClothSim
{
	// SprintNode definition
	// --------------------------------------------
	struct sParticle
	{
		glm::vec3 V;// velocity
		glm::vec3 P;// position
		glm::vec3 pP; // previous position
		bool isFixed = false;

		sParticle():V(glm::vec3(0)), P(glm::vec3(0)), pP(glm::vec3(0)) {}
		sParticle(glm::vec3 initialP) : V(glm::vec3(0)), P(initialP), pP(initialP) {}
	};

	// rendering data
	// --------------------------------------------
#define CLOTH_RESOLUTION 5
#define VC CLOTH_RESOLUTION * CLOTH_RESOLUTION

	extern sParticle g_particles[VC];

	// Clock wise from struct-shear-bend
	// --------------------------------------------
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



// stiff and damping for spring particles
#define STRUCT_STIFF 30
#define STRUCT_DAMP	-0.5
#define SHEAR_STIFF 30
#define SHEAR_DAMP	-0.5
#define BEND_STIFF 15
#define BEND_DAMP	-0.5

#define MASS 1.f // 1 kg
#define GRAVITY glm::vec3(0, -9.8f, 0);
#define GRAVITY_DAMPING -0.30
#define DEFAULT_DAMPING -0.25

// Using discrete time step
	void UpdateSprings(const float dt);

}

#endif // !SIM_PARAMS_DEFINED

