#include "OctTree.h"
#include "Graphics/EnvironmentCaptureManager.h"

void sOctTree::InitializeTree(const cBox& i_initialVolume, const std::vector<PROBE*>& i_initialProbeList)
{
	Volume = i_initialVolume;
	m_probeList = i_initialProbeList;

	BuildTree();
}

void sOctTree::BuildTree()
{
	// If this node is a leaf node, no need to build the tree any more
	if (m_probeList.size() <= 1) return;
}

sOctTree* sOctTree::LocatePoint(const glm::vec3& i_POI)
{
	return nullptr;
}

std::vector<sOctTree::PROBE*> sOctTree::GetIntersectProbes(const glm::vec3& i_POI)
{
	std::vector<Graphics::EnvironmentCaptureManager::sCaptureProbes*> _result;

	// 1. locate the leaf node where this POI is.
	sOctTree* _node = LocatePoint(i_POI);
	// 2. check collision of this point with all objects inside this node.
	for (auto it: _node->m_probeList)
	{
		// if this point is inside the Bounding volume, requested result
		if (it->BV.Intersect(i_POI)) _result.push_back(it);
	}

	return _result;
}
