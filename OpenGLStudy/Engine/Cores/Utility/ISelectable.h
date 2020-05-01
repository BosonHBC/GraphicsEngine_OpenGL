#pragma once
#include <map>

class ISelectable
{
public:
	static uint32_t s_selectableCount;
	static std::map<uint32_t, void*> s_selectableList;

	ISelectable() {};
	virtual ~ISelectable() {}
	ISelectable(const ISelectable& i_rhs)
	{
		SelectableID = i_rhs.SelectableID;
	}
	ISelectable& operator = (const ISelectable& i_rhs)
	{
		SelectableID = i_rhs.SelectableID;
		return *this;
	}

	void IncreamentSelectableCount()
	{
		s_selectableCount++;
		SelectableID = s_selectableCount;
		if (s_selectableList.find(SelectableID) == s_selectableList.end())
			s_selectableList.insert(std::pair<uint32_t, void*>(SelectableID, this));
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

	uint32_t SelectableID = -1;
private:

};

__declspec(selectany) uint32_t ISelectable::s_selectableCount = 0;

__declspec(selectany) std::map<uint32_t, void*> ISelectable::s_selectableList;
