// AdvancedUserInput.cpp : Defines the functions for the static library.
//
#include <cstdint>
#include "AdvancedUserInput.h"


Assets::cAssetManager< UserInput::AdvancedUserInput>  UserInput::AdvancedUserInput::s_manager;


namespace UserInput {

	bool AdvancedUserInput::Load(const std::string i_path, AdvancedUserInput*& o_ptr)
	{
		auto result = true;

		std::map<const char*, KeyCodes::eKeyCodes> actionMappings;
		std::map<const char*, FAxisBrother> axisMappings;

/*
		auto currentOffset = reinterpret_cast<uintptr_t>(inFile.data);

		uint8_t actionMappingCount = *reinterpret_cast<uint8_t*>(currentOffset);
		currentOffset += sizeof(uint8_t);
		uint8_t axisMappingCount = *reinterpret_cast<uint8_t*>(currentOffset);
		currentOffset += sizeof(uint8_t);
		for (int i = 0; i < actionMappingCount; ++i)
		{
			const auto actionName = reinterpret_cast<const char*>(currentOffset);
			currentOffset += strlen(actionName) + 1;
			const auto _keyCode = *reinterpret_cast<KeyCodes::eKeyCodes*>(currentOffset);
			currentOffset += sizeof(KeyCodes::eKeyCodes);

			actionMappings.insert(std::pair<const char*, KeyCodes::eKeyCodes>(actionName, _keyCode));
		}
		for (int i = 0; i < axisMappingCount; ++i)
		{
			const auto axisName = reinterpret_cast<const char*>(currentOffset);
			currentOffset += strlen(axisName) + 1;
			const auto negativeKeyCode = *reinterpret_cast<KeyCodes::eKeyCodes*>(currentOffset);
			currentOffset += sizeof(KeyCodes::eKeyCodes);
			const auto positiveKeyCode = *reinterpret_cast<KeyCodes::eKeyCodes*>(currentOffset);
			currentOffset += sizeof(KeyCodes::eKeyCodes);

			axisMappings.insert(std::pair<const char*, FAxisBrother>(axisName, FAxisBrother(negativeKeyCode, positiveKeyCode)));

		}*/

		o_ptr = new AdvancedUserInput();
		result = o_ptr->ReadMappings(actionMappings, axisMappings);

		return result;
	}

	bool AdvancedUserInput::ReadMappings(std::map<const char*, KeyCodes::eKeyCodes> i_actionMappings, std::map<const char*, FAxisBrother> i_axisMappings)
	{
		auto result = true;
		for (auto it = i_actionMappings.begin(); it != i_actionMappings.end(); ++it)
		{
			AddActionKeyPairToMap(it->first, it->second);
		}
		for (auto it = i_axisMappings.begin(); it != i_axisMappings.end(); ++it)
		{
			KeyCodes::eKeyCodes* axisPair = new KeyCodes::eKeyCodes[2]{
				it->second.negative_key,
				it->second.positive_key
			};
			AddAxisKeyPairToMap(it->first, axisPair);

		}
		return result;
	}

	bool AdvancedUserInput::RemoveUserInput(AdvancedUserInput*& o_ptr)
	{
		if (o_ptr)
			delete o_ptr;
		return true;
	}

	bool AdvancedUserInput::LoadFileFromLUA(const std::string i_path, std::map<const char*, KeyCodes::eKeyCodes>& i_axisMapping, std::map<const char*, FAxisBrother>& i_actionMapping)
	{
		auto result = true;

		return result;
	}

	void AdvancedUserInput::AddActionKeyPairToMap(const char* i_actionName, const KeyCodes::eKeyCodes& i_keycode)
	{
		m_actionKeyMap.insert({ i_actionName, i_keycode });
		if (!IsKeyExisted(i_keycode)) {
			m_keyStatusMap.insert({ i_keycode, false });
		}
	}

	void  AdvancedUserInput::AddAxisKeyPairToMap(const char* i_axisName, KeyCodes::eKeyCodes* i_keycodes)
	{
		m_axisKeyMap.insert(
			{
				i_axisName,
				i_keycodes
			}
		);
		if (!IsKeyExisted(i_keycodes[0])) {
			m_keyStatusMap.insert({ i_keycodes[0], false });
		}
		if (!IsKeyExisted(i_keycodes[1])) {
			m_keyStatusMap.insert({ i_keycodes[1], false });
		}
	}

	AdvancedUserInput::~AdvancedUserInput()
	{
		// clean up key code pair
		for (auto it = m_axisKeyMap.begin(); it != m_axisKeyMap.end(); ++it)
		{
			if (it->second != nullptr) {
				KeyCodes::eKeyCodes* tempKeycode = it->second;
				delete tempKeycode;
				it->second = nullptr;
			}
		}
		m_axisKeyMap.clear();
	}

	bool AdvancedUserInput::IsKeyExisted(const KeyCodes::eKeyCodes& i_key) const
	{
		return m_keyStatusMap.find(i_key) != m_keyStatusMap.end();
	}

	void AdvancedUserInput::UpdateInput()
	{
		// iterate through all action bindings
		{
			for (auto it = m_actionBindings.begin(); it != m_actionBindings.end(); ++it)
			{
				auto key = it->GetBoundKey();
				bool isKeyDown = IsKeyDown(key);

				// According to the input event type to do input check
				switch (it->GetInputType())
				{
				case IT_OnPressed:
					if (isKeyDown && !m_keyStatusMap[key]) {
						it->Execute();
					}
					break;
				case IT_OnReleased:
					if (!isKeyDown && m_keyStatusMap[key]) {
						it->Execute();
					}
					break;
				case IT_OnHold:
					if (isKeyDown) {
						it->Execute();
					}
					break;
				default:
					break;
				}

				// TODO:  More useful events should be implemented here later
				// ...
			}
		}
		// iterate through all axis bindings
		{
			for (auto it = m_axisBindings.begin(); it != m_axisBindings.end(); ++it)
			{
				auto keyL = it->GetBoundKey()[0];
				auto keyR = it->GetBoundKey()[1];

				bool isKeyDownL = IsKeyDown(keyL);
				bool isKeyDownR = IsKeyDown(keyR);

				// Currently only raw data
				float value = 0;
				if (isKeyDownL && !isKeyDownR) value = -1;
				if (!isKeyDownL && isKeyDownR) value = 1;

				it->Execute(value);
			}
		}

		// Update key state for binding keys
		for (auto it = m_keyStatusMap.begin(); it != m_keyStatusMap.end(); ++it)
		{
			it->second = IsKeyDown(it->first);
		}
	}

	bool AdvancedUserInput::IsActionPressed(const char* i_actionName)
	{
		if (!IsActionNameValid(i_actionName)) return false;
		const short isKeyDownMask = ~1;
		const auto keyState = GetKeyState(m_actionKeyMap[i_actionName]);
		return (keyState & isKeyDownMask) != 0;
	}

	bool AdvancedUserInput::IsActionNameValid(const char* i_actionName)
	{
		bool isValid = m_actionKeyMap.find(i_actionName) != m_actionKeyMap.end();
		return isValid;
	}

	bool AdvancedUserInput::IsAxisNameValid(const char* i_axisName)
	{
		bool isValid = m_axisKeyMap.find(i_axisName) != m_axisKeyMap.end();
		return isValid;
	}

	bool AdvancedUserInput::IsActionReleased(const char* i_actionName)
	{
		if (!IsActionNameValid(i_actionName)) return false;
		const short isKeyDownMask = ~1;
		const auto keyState = GetKeyState(m_actionKeyMap[i_actionName]);
		return (keyState & isKeyDownMask) == 0;
	}

	// -------------------------------
	// Action Binding Definition

	ActionBinding::ActionBinding(const char* i_actionName, std::map<std::string, KeyCodes::eKeyCodes> i_actionKeyMap, InputType i_inputType)
	{
		m_boundKeyCode = i_actionKeyMap[i_actionName];
		m_InputType = i_inputType;
		m_boundDelegate = nullptr;
	}

	AxisBinding::AxisBinding(const char* i_axisName, std::map<std::string, KeyCodes::eKeyCodes*> i_axisKeyMap)
	{
		m_boundKeyCode[0] = i_axisKeyMap[i_axisName][0];
		m_boundKeyCode[1] = i_axisKeyMap[i_axisName][1];
		m_boundDelegate = nullptr;
	}

}