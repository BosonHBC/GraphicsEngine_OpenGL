/*
	This OcTree right now is only for spherical collision detection in Environment capturing
	No more further usage right now, so there is no abstraction
*/
#pragma once
#include "Math/Shape/Box.h"
#include <vector>
// Forward deceleration
namespace Graphics {
	namespace EnvironmentCaptureManager {
		struct sCaptureProbes;
	}
}
// Only allow maximum 4 level sub-division
#define MAX_DEPTH 4
struct sOctTree
{
	typedef Graphics::EnvironmentCaptureManager::sCaptureProbes PROBE; // for simplicity

	// The volume of this node, radius should be the exponent of 2, the root radius is predefined
	cBox Volume;

	/** Construction functions */
	void InitializeTree(const cBox& i_initialVolume, const std::vector<PROBE*>& i_initialProbeList);
	void BuildTree();
	void CleanUp();

	/** Query functions */
	// Find out the leaf node this point is in
	sOctTree* LocatePoint(const glm::vec3& i_POI);
	std::vector<PROBE*> GetIntersectProbes(const glm::vec3& i_POI);

private:
	int m_currentDepth = 0; // Start with level 0, which is the root node
	sOctTree* m_parent = nullptr;
	// 8 child nodes
	sOctTree* m_childNode[8] = { nullptr };
	// All probe that is in this volume
	std::vector<PROBE*> m_probeList;
};