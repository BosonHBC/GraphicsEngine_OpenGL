#include "Cores/Core.h"
#include "SimulationParams.h"
#include "Math/Shape/Sphere.h"
#include <random>
namespace ClothSim
{
	float * g_vertexData;
	const int vertSIze = 14; // position, textureCoords, normal, tangent, bitangent
	std::vector<unsigned int> g_indexData;
	const int numOfTriangles = (CLOTH_RESOLUTION - 1)* (CLOTH_RESOLUTION - 1) * 2;

	sParticle g_particles[VC];
	const float g_structRestLen = static_cast<float>(CLOTH_LENGTH) / static_cast<float>(CLOTH_RESOLUTION - 1);
	const float g_shearRestLen = g_structRestLen * sqrt(2.f);
	const float g_blendRestLen = g_structRestLen * 2.f;
	const int R = CLOTH_RESOLUTION;
	glm::vec3 springForce(glm::vec3 myVel, glm::vec3 neiVel, glm::vec3 myPosition, glm::vec3 neiPosition, float restLength, float stiffness, float damping);
	void fixStretchConstrain(const int vertexIdx, glm::vec3& io_adjustedPos, const float threshold, const float divider);
	void HandleSphereCollision(const cSphere& i_sph, int idx, glm::vec3& io_nextPos);
	void HandleFriction(glm::vec3& io_force, const glm::vec3& i_normal, const glm::vec3& i_velocity);

	glm::vec3 g_positionData[VC] = { glm::vec3(0) };

	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;

#define FRICTION_COEFFICENT 0.1f
#define TOUCH_DIST_THRESHOLD 1.f
#define SPHERE_COLLISION_BIAS 1.1f
	const glm::vec3 g_floorPlane = glm::vec3(0, 5, 0);
	cSphere g_sphere(glm::vec3(0, 0, -150), 100.f);
	const float inSphereDist = sqrt(pow(g_sphere.r(), 2) + pow(g_structRestLen / 2, 2));

	void InitializeNeghbors()
	{
		for (int i = 0; i < VC; ++i)
		{
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
		}

		const int numOfVertices = VC;
		g_vertexData = new float[14 * VC];
		g_indexData.reserve(numOfTriangles * 3);

		const int indexPerQuad = 6;
		// For each quad, start from the top-left corner
		for (int i = 0; i < CLOTH_RESOLUTION - 1; ++i)
		{
			for (int j = 0; j < CLOTH_RESOLUTION - 1; ++j)
			{
				int idx = i * CLOTH_RESOLUTION + j;
				// first triangle
				g_indexData.push_back(idx);
				g_indexData.push_back(idx + CLOTH_RESOLUTION);
				g_indexData.push_back(idx + 1);
				// second triangle
				g_indexData.push_back(idx + 1);
				g_indexData.push_back(idx + CLOTH_RESOLUTION);
				g_indexData.push_back(idx + CLOTH_RESOLUTION + 1);
			}
		}

	}

	void UpdateV3Data(int idx, const glm::vec3* i_data, int stride)
	{
		// locate in which vertex and add the stride
		memcpy(g_vertexData + vertSIze * idx + stride, i_data, sizeof(float) * 3);
	}
	void UpdateV2Data(int idx, const glm::vec2* i_data, int stride)
	{
		// locate in which vertex and add the stride
		memcpy(g_vertexData + vertSIze * idx + stride, i_data, sizeof(float) * 2);
	}

	glm::vec3 glmWindForce = glm::vec3(0, 0, -10.f);
	glm::vec3 windForce(glm::vec3 normal)
	{
		glm::vec3 n = normal;
		return glm::dot(n, glmWindForce) * glmWindForce;
	}

	void UpdateSprings(const float dt)
	{
		for (int i = 0; i < VC; ++i)
		{
			const int r = i / R; // current row
			const int c = i % R; // current Column

			glm::vec2 texcoord = glm::vec2(
				c / static_cast<float>(CLOTH_RESOLUTION - 1),
				r / static_cast<float>(CLOTH_RESOLUTION - 1)
			);
			UpdateV2Data(i, &texcoord, 3);

			// calculate velocity;
			glm::vec3 currentV = g_particles[i].V;

			// overall force for this node to be applied
			glm::vec3 force = glm::vec3(0.0);

			// Calculate force from valid neighbors
			for (int j = 0; j < 12; ++j)
			{
				const sNeighborParticles& _neighbor = g_particles[i].neighbor[j];
				if (_neighbor.idx == NO_Neighbor)
					continue;
				glm::vec3 neiPosition = g_particles[_neighbor.idx].P;
				glm::vec3 neiVel = g_particles[_neighbor.idx].V;

				force += springForce(currentV, neiVel, g_particles[i].P, neiPosition, _neighbor.restLength, _neighbor.stiff, _neighbor.damp);
			}

			// calculate normal
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;
			{
				const auto rightIdx = g_particles[i].neighbor[Struct_Right].idx;
				const auto leftIdx = g_particles[i].neighbor[Struct_Left].idx;
				const auto upIdx = g_particles[i].neighbor[Struct_Up].idx;
				const auto downIdx = g_particles[i].neighbor[Struct_Down].idx;
				if (c < CLOTH_RESOLUTION - 1)
				{
					tangent = glm::normalize(g_particles[rightIdx].P - g_particles[i].P);
					if (r < CLOTH_RESOLUTION - 1)
					{
						bitangent = glm::normalize(g_particles[downIdx].P - g_particles[i].P);
						normal += glm::normalize(glm::cross(bitangent, tangent));
					}
					if (r > 0)
					{
						bitangent = glm::normalize((g_particles[i].P - g_particles[upIdx].P));
						normal += glm::normalize(glm::cross(bitangent, tangent));
					}
				}
				if (c > 0)
				{
					tangent = glm::normalize(g_particles[leftIdx].P - g_particles[i].P);
					if (r < CLOTH_RESOLUTION - 1)
					{
						bitangent = glm::normalize(g_particles[downIdx].P - g_particles[i].P);
						normal += glm::normalize(glm::cross(bitangent, tangent));
					}
					if (r > 0)
					{
						bitangent = glm::normalize((g_particles[i].P - g_particles[upIdx].P));
						normal += glm::normalize(glm::cross(bitangent, tangent));
					}
				}
				UpdateV3Data(i, &normal, 5);
				UpdateV3Data(i, &tangent, 8);
				UpdateV3Data(i, &bitangent, 11);
			}
			normal = glm::normalize(normal);

			// Add gravity force
			force += GRAVITY * MASS + GRAVITY_DAMPING * currentV;

			// handle friction for sphere
			{
				glm::vec3 centerToP = g_particles[i].P - g_sphere.c();
				float dist = glm::length(centerToP);
				if (dist <= inSphereDist)
				{
					HandleFriction(force, centerToP / dist, currentV);
				}
			}


			// Verlet integration
			glm::vec3 a = glm::vec3(0); // a stands for acceleration
			if (!g_particles[i].isFixed)
			{
				a = force / MASS;
			}
			glm::vec3 nextPosition = g_particles[i].P + currentV * dt + a * dt * dt;

			// Handle sphere collision
			float nextPosToCenter_dist = glm::distance(nextPosition, g_sphere.c());
			if (nextPosToCenter_dist < g_sphere.r())
			{
				HandleSphereCollision(g_sphere, i, nextPosition);
			}

			g_positionData[i] = nextPosition;
		}

		// update position info
		for (int i = 0; i < VC; ++i)
		{
			g_particles[i].pP = g_particles[i].P;
			if (!g_particles[i].isFixed && g_positionData[i].y > g_floorPlane.y)
			{
				g_particles[i].P = g_positionData[i];
			}
			g_particles[i].V = (g_particles[i].P - g_particles[i].pP) / dt;

			UpdateV3Data(i, &g_particles[i].P, 0);
		}


		// Inverse dynamic procedural
		if (false)
		{
			for (int i = 0; i < VC; ++i)
			{
				glm::vec3 adjustedPos = g_particles[i].P;
				if (!g_particles[i].isFixed)
				{
					const auto structUpIdx = g_particles[i].neighbor[Struct_Up].idx;
					if (structUpIdx != NO_Neighbor)
						fixStretchConstrain(structUpIdx, adjustedPos, g_structRestLen * 1.1f, 1.0f);

					const auto structRightIdx = g_particles[i].neighbor[Struct_Right].idx;
					if (structRightIdx != NO_Neighbor)
						fixStretchConstrain(structRightIdx, adjustedPos, g_structRestLen * 1.1f, 2.0f);

					const auto structLeftIdx = g_particles[i].neighbor[Struct_Left].idx;
					if (structLeftIdx != NO_Neighbor)
						fixStretchConstrain(structLeftIdx, adjustedPos, g_structRestLen * 1.1f, 2.0f);

					const auto Shear0Idx = g_particles[i].neighbor[Shear_0].idx;
					if (Shear0Idx != NO_Neighbor)
						fixStretchConstrain(Shear0Idx, adjustedPos, g_shearRestLen * 1.2f, 1.0f);

					const auto Shear1Idx = g_particles[i].neighbor[Shear_1].idx;
					if (Shear1Idx != NO_Neighbor)
						fixStretchConstrain(Shear1Idx, adjustedPos, g_shearRestLen * 1.2f, 1.0f);

					g_positionData[i] = adjustedPos;
				}
			}

			// update position info and velocity info
			for (int i = 0; i < VC; ++i)
			{
				if (!g_particles[i].isFixed)
				{
					g_particles[i].P = g_positionData[i];
					g_particles[i].V = (g_particles[i].P - g_particles[i].pP) / dt;
					UpdateV3Data(i, &g_particles[i].P, 0);
				}
			}
		}


	}

	void MoveFixedNode(const glm::vec3& i_deltaPosition)
	{
		for (int i = 0; i < CLOTH_RESOLUTION; ++i)
		{
			if (g_particles[i].isFixed)
				g_particles[i].P += i_deltaPosition;
		}
	}

	void ScaleFixedNode(const glm::vec3& i_deltaPosition)
	{
		for (int i = 0; i < CLOTH_RESOLUTION; ++i)
		{
			if (g_particles[i].isFixed)
				g_particles[i].P += i_deltaPosition * (1.f - i / static_cast<float>(CLOTH_RESOLUTION - 1));
		}

	}

	void CleanUpData()
	{
		safe_delete(g_vertexData);
	}

	float* GetVertexData()
	{
		return g_vertexData;
	}

	const std::vector<unsigned int>& GetIndexData()
	{
		return g_indexData;
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

	void fixStretchConstrain(const int vertexIdx, glm::vec3& io_adjustedPos, const float threshold, const float divider)
	{
		glm::vec3 otherPosition = g_particles[vertexIdx].P;
		glm::vec3 exceedPos = io_adjustedPos - otherPosition;
		float exceedDist = glm::length(exceedPos);
		if (exceedDist > threshold) {
			glm::vec3 fix = glm::normalize(-exceedPos) * (exceedDist - threshold) / divider;
			io_adjustedPos += fix;
		}
	}

	void HandleSphereCollision(const cSphere& i_sph, int idx, glm::vec3& io_nextPos)
	{

		glm::vec3 rayOrigin = g_particles[idx].P;
		glm::vec3 rayDir = io_nextPos - rayOrigin;
		glm::vec3 rayDir_Norm = glm::normalize(rayDir);
		float t1 = 0, t2 = 0; // t1 is smaller
		if (ECT_Overlap == i_sph.InverectRay(rayOrigin, rayDir_Norm, t1, t2))
		{
			glm::vec3 interectionPoint = rayOrigin + t1 * rayDir_Norm;
			float offsetDist = inSphereDist;
			if (idx / CLOTH_RESOLUTION > 0)
			{
				float distToUpNode = glm::distance(g_particles[idx - CLOTH_RESOLUTION].P, interectionPoint);
				offsetDist = sqrt(pow(g_sphere.r(), 2) + pow(distToUpNode / 2, 2));
			}
				
			io_nextPos = interectionPoint + (offsetDist - g_sphere.r()) * glm::normalize(interectionPoint - g_sphere.c()) * SPHERE_COLLISION_BIAS;
			//i_sph.c() + SPHERE_COLLISION_BIAS * (interectionPoint - i_sph.c());
		}
	}

	void HandleFriction(glm::vec3& io_force, const glm::vec3& i_normal, const glm::vec3& i_velocity)
	{
		// Add friction 
		glm::vec3 fNorm = i_normal;
		glm::vec3 fTangent = glm::normalize(glm::cross(glm::cross(fNorm, io_force), fNorm));

		float fVertical = glm::dot(io_force, -fNorm);
		float fHori = glm::dot(io_force, fTangent);
		int frictionDir = (glm::dot(i_velocity, fTangent) > 0) ? 1 : -1;
		// if the overall force is going down, calculate friction
		if (fVertical > 0.0f)
		{
			float fFriction = fVertical * FRICTION_COEFFICENT;
			io_force = (fHori + frictionDir * fFriction) * fTangent/* + i_velocity * -0.75f*/;
		}

	}
}
