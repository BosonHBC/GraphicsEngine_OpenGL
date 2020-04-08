#include "SimulationParams.h"
namespace ClothSim
{
	sParticle g_particles[VC];
	const float g_structRestLen = static_cast<float>(CLOTH_LENGTH) / static_cast<float>(CLOTH_RESOLUTION - 1);
	const float g_shearRestLen = g_structRestLen * sqrt(2.f);
	const float g_blendRestLen = g_structRestLen * 2.f;
	const int R = CLOTH_RESOLUTION;
	glm::vec3 springForce(glm::vec3 myVel, glm::vec3 neiVel, glm::vec3 myPosition, glm::vec3 neiPosition, float restLength, float stiffness, float damping);
	void fixStretchConstrain(const int vertexIdx, glm::vec3& io_adjustedPos,const float threshold);
	glm::vec3 g_positionData[VC] = {glm::vec3(0)};
	glm::vec3 g_prevVelocity[VC] = { glm::vec3(0) };
	void UpdateSprings(const float dt)
	{
		for (int i = 0; i < VC; ++i)
		{
			// calculate velocity;
			glm::vec3 currentV = (g_particles[i].P - g_particles[i].pP) / dt;

			// overall force for this node to be applied
			glm::vec3 force = glm::vec3(0.0);

			// Get neighbors

			const int r = i / R; // current row
			const int c = i % R; // current Column
			// Set up neighbors
			{
				if (r > 0) {
					g_particles[i].neighbor[Struct_Up].SetIdx(Struct_Up, i - R);
					if (r - 1 > 0)
						g_particles[i].neighbor[Bend_Up].SetIdx(Bend_Up, i - 2 * R);
				}
				if (r < R - 1) {
					g_particles[i].neighbor[Struct_Down].SetIdx(Struct_Down, i + R);
					if (r + 1 < R - 1)
						g_particles[i].neighbor[Bend_Down].SetIdx(Bend_Down, i + 2 * R);
				}
				if (c > 0) {
					g_particles[i].neighbor[Struct_Left].SetIdx(Struct_Left, i - 1);
					if (c - 1 > 0)
						g_particles[i].neighbor[Bend_Left].SetIdx(Bend_Left, i - 2);
				}
				if (c < R - 1) {
					g_particles[i].neighbor[Struct_Right].SetIdx(Struct_Right, i + 1);
					if (c + 1 < R - 1)
						g_particles[i].neighbor[Bend_Right].SetIdx(Bend_Right, i + 2);
				}
				if (r > 0 && c > 0)
					g_particles[i].neighbor[Shear_0].SetIdx(Shear_0, i - R - 1);
				if (r > 0 && c < R - 1)
					g_particles[i].neighbor[Shear_1].SetIdx(Shear_1, i - R + 1);
				if (r < R - 1 && c > 0)
					g_particles[i].neighbor[Shear_3].SetIdx(Shear_3, i + R - 1);
				if (r < R - 1 && c < R - 1)
					g_particles[i].neighbor[Shear_2].SetIdx(Shear_2, i + R + 1);
			}

			// Calculate force from valid neighbors
			for (int j = 0; j < 12; ++j)
			{
				if(g_particles[i].neighbor[j].idx == NO_Neighbor)
					continue;
				glm::vec3 neiPosition = g_particles[g_particles[i].neighbor[j].idx].P;
				glm::vec3 neiVel = g_particles[g_particles[i].neighbor[j].idx].V;

				force += springForce(currentV, neiVel, g_particles[i].P, neiPosition, g_particles[i].neighbor[j].restLength, g_particles[i].neighbor[j].stiff, g_particles[i].neighbor[j].damp);
			}

			// Add gravity force
			force += GRAVITY * MASS +  GRAVITY_DAMPING * currentV;

			// Verlet integration
			glm::vec3 a = glm::vec3(0); // a stands for acceleration
			if (!g_particles[i].isFixed)
			{
				a = force /  MASS;
			}
			glm::vec3 nextPosition = g_particles[i].P + currentV * dt + a * dt * dt;
			glm::vec3 adjustedPos = nextPosition;
			if (!g_particles[i].isFixed)
			{
				const auto structLeftIdx = g_particles[i].neighbor[Struct_Left].idx;
				if (structLeftIdx != NO_Neighbor)
					fixStretchConstrain(structLeftIdx, adjustedPos, g_structRestLen * 1.1f);

				const auto structRightIdx = g_particles[i].neighbor[Struct_Right].idx;
				if (structRightIdx != NO_Neighbor)
					fixStretchConstrain(structRightIdx, adjustedPos, g_structRestLen * 1.1f);

				const auto structUpIdx = g_particles[i].neighbor[Struct_Up].idx;
				if (structUpIdx != NO_Neighbor)
					fixStretchConstrain(structUpIdx, adjustedPos, g_structRestLen * 1.1f);

				const auto Shear0Idx = g_particles[i].neighbor[Shear_0].idx;
				if (Shear0Idx != NO_Neighbor)
					fixStretchConstrain(Shear0Idx, adjustedPos, g_shearRestLen * 1.2f);

				const auto Shear1Idx = g_particles[i].neighbor[Shear_1].idx;
				if (Shear1Idx != NO_Neighbor)
					fixStretchConstrain(Shear1Idx, adjustedPos, g_shearRestLen * 1.2f);
			}

			g_positionData[i] = adjustedPos;
			g_prevVelocity[i] = currentV;
		} 
		
		// update position info
		for (int i = 0; i < VC; ++i)
		{
			g_particles[i].V = g_prevVelocity[i];
			g_particles[i].pP = g_particles[i].P;
			if (!g_particles[i].isFixed)
				g_particles[i].P = g_positionData[i];
		}

/*
		// adjust data
		for (int i = 0; i < VC; ++i)
		{
			glm::vec3 adjustedPos = g_particles[i].P;
			//stretching constraint, fix the distance
			
		}
		// update position info
		for (int i = 0; i < VC; ++i)
		{
			if (!g_particles[i].isFixed)
				g_particles[i].P = g_positionData[i];
		}*/
	}

	void MoveFixedNode(const glm::vec3& i_deltaPosition)
	{
		g_particles[0].P += i_deltaPosition;
		g_particles[CLOTH_RESOLUTION - 1].P += i_deltaPosition;
	}

	glm::vec3 springForce(glm::vec3 myVel, glm::vec3 neiVel, glm::vec3 myPosition, glm::vec3 neiPosition, float restLength, float stiffness, float damping)
	{
		glm::vec3 deltaP = myPosition - neiPosition;
		glm::vec3 deltaV = myVel - neiVel;
		float dist = glm::length(deltaP);
		float stiff = (restLength - dist) *stiffness;
		float damp = damping * (dot(deltaV, deltaP) / dist);
		return (stiff + damp) * normalize(deltaP);
	}
	void fixStretchConstrain(const int vertexIdx, glm::vec3& io_adjustedPos,const float threshold)
	{
		glm::vec3 otherPosition = g_particles[vertexIdx].P;
		glm::vec3 exceedPos = io_adjustedPos - otherPosition;
		float exceedDist = glm::length(exceedPos);
		if (exceedDist > threshold) {
			glm::vec3 fix = glm::normalize(-exceedPos) * (exceedDist - threshold) / 2.f;
			io_adjustedPos += fix;
		}
	}
}
