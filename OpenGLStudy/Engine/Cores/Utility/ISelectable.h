#pragma once
#include <map>
class cTransform;
class ISelectable
{
public:
	static uint32_t s_selectableCount;
	static std::map<uint32_t, ISelectable*> s_selectableList;
	bool static IsValid(uint32_t i_id) { return i_id < 256 && i_id > 0; }

	ISelectable() {};
	virtual ~ISelectable() {}
	ISelectable(const ISelectable& i_rhs) = default;
	ISelectable& operator = (const ISelectable& i_rhs) = default;


	void IncreamentSelectableCount()
	{
		s_selectableCount++;
		SelectableID = s_selectableCount;
		if (s_selectableList.find(SelectableID) == s_selectableList.end())
			s_selectableList.insert({ SelectableID, this });
		else
			assert(false);
	}

	void DecreamentSelectableCount() {
		if (s_selectableList.find(SelectableID) != s_selectableList.end())
			s_selectableList.erase(SelectableID);
		if (s_selectableCount > 0)
			s_selectableCount--;
		SelectableID = -1;
	}

	virtual void OnSelected() {};
	virtual void OnDeSelected() {};

	virtual bool GetBoundTransform(cTransform *& o_transform) { return false; }

	uint32_t SelectableID = -1;
private:

};

__declspec(selectany) uint32_t ISelectable::s_selectableCount = 0;

__declspec(selectany) std::map<uint32_t, ISelectable*> ISelectable::s_selectableList;
