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
	// If this node is a leaf node or If this is the deepest level, no need to build the tree any more
	if (m_probeList.size() <= 1 || m_currentDepth >= MAX_DEPTH) return;

	cBox _8Boxes[8];
	std::vector<PROBE*> _childInitialProbeList[8];
	// 1. generate boxes for child nodes
	{
		glm::vec3 _diagonalQuater = Volume.diagonal() / 4.f; // half dimension of sub-divided box
		glm::vec3 _center = Volume.center();
		cBox _octSubBox = Volume.getOctSubBox(); // a box with 1/8 size of Volume box and the same center

		_8Boxes[0] = _octSubBox + _diagonalQuater * glm::vec3(-1, -1, -1);
		_8Boxes[1] =_octSubBox +_diagonalQuater * glm::vec3(1, -1, -1);
		_8Boxes[2] =_octSubBox +_diagonalQuater * glm::vec3(-1, 1, -1);
		_8Boxes[3] =_octSubBox +_diagonalQuater * glm::vec3(1, 1, -1);
		_8Boxes[4] =_octSubBox +_diagonalQuater * glm::vec3(-1, -1, 1);
		_8Boxes[5] =_octSubBox +_diagonalQuater * glm::vec3(1, -1, 1);
		_8Boxes[6] =_octSubBox +_diagonalQuater * glm::vec3(-1, 1, 1);
		_8Boxes[7] =_octSubBox +_diagonalQuater * glm::vec3(1, 1, 1);

	}
	// 2. determine if there is object in this node
	{
		for (auto i = 0; i < 8; ++i)
		{
			_childInitialProbeList->reserve(m_probeList.size());// reserve the same memory as this->probelist 
			 // loop though probe list to see if a probe is intersect with the child node's volume, 
			// if it is overlapping, this probe is in the probe list of this child node
			for (auto it : m_probeList)
			{
				if (_8Boxes[i].Intersect(&it->BV) == ECT_Overlap)
				{
					_childInitialProbeList[i].push_back(it); // add to the child node
				}
			}
		}
	}
	// 3. allocate memory for child nodes if the child node has at least one item
	for (auto i = 0; i < 8; ++i)
	{
		if (_childInitialProbeList[i].size() > 0)
		{
			m_childNode[i] = new sOctTree();
			// Update depth
			m_childNode[i]->m_currentDepth = m_currentDepth + 1;
			// Update parent
			m_childNode[i]->m_parent = this;
			m_childNode[i]->InitializeTree(_8Boxes[i], _childInitialProbeList[i]);
			printf("OctTree level[%d][%d] building done, probe list count: %d.\n", m_currentDepth, i,m_probeList.size());
		}
	}
	
}

void sOctTree::CleanUp()
{
	m_probeList.clear();
	for (auto i = 0; i < 8; ++i)
	{
		sOctTree* _child = m_childNode[i];
		if (_child != nullptr) {
			_child->CleanUp();
			safe_delete(_child);
		}

	}
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
	for (auto it : _node->m_probeList)
	{
		// if this point is inside the Bounding volume, requested result
		if (it->BV.Intersect(i_POI)) _result.push_back(it);
	}

	return _result;
}
