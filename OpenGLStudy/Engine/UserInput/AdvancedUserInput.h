#pragma once
#include <vector>
#include <map>
#include "InputDelegate.h"
#include <Engine/Assets/Handle.h>
#include <Engine/Assets/AssetManager.h>
#include "Engine/UserInput/UserInput.h"
#include "assert.h"

#define IT_OnPressed 1
#define IT_OnReleased 2
#define IT_OnHold 3

struct FAxisBrother {
	FAxisBrother(UserInput::KeyCodes::eKeyCodes i_nKey, UserInput::KeyCodes::eKeyCodes i_pKey) : negative_key(i_nKey), positive_key(i_pKey) {}
	UserInput::KeyCodes::eKeyCodes negative_key;
	UserInput::KeyCodes::eKeyCodes positive_key;
};
namespace UserInput {

	// -------------------------------
	// Input Type
	// 5 common Input types for keyboard
	enum EInputType
	{
		EIT_OnPressed = 1,
		EIT_OnReleased = 2,
		EIT_OnHold = 3,
		//EIT_OnDoubleClick = 4,
		//EIT_OnRepeat = 5
	};
	typedef uint8_t InputType;

	// -------------------------------
	// Action Binding
	// Action Binding struct stores binding info like axis name and the delegate function
	struct ActionBinding {
	public:
		/** Constructor for ActionBinding struct*/
		ActionBinding()
			:m_boundKeyCode(), m_InputType(IT_OnPressed), m_boundDelegate(nullptr)
		{}

		ActionBinding(const char* i_actionName, std::map<std::string, KeyCodes::eKeyCodes> i_actionKeyMap, InputType i_inputType);

		/** Execute the binding functions*/
		void Execute() { m_boundDelegate->Execute(); }

		/** Setters */
		void SetDelegate(IDelegateHandler<>* i_boundDelegate) { m_boundDelegate = i_boundDelegate; }
		/** Getters */
		KeyCodes::eKeyCodes GetBoundKey() const { return m_boundKeyCode; }
		InputType GetInputType() const { return m_InputType; }
	private:
		KeyCodes::eKeyCodes m_boundKeyCode;
		InputType m_InputType;
		// Delegate with no input parameter
		IDelegateHandler<>* m_boundDelegate;
	};

	// -------------------------------
	// Axis Binding
	// Axis Binding struct stores binding info like axis name and the delegate function
	struct AxisBinding {
		/** Default Constructor for AxisBinding struct*/
		AxisBinding()
			:m_boundKeyCode(), m_boundDelegate(nullptr)
		{}

		AxisBinding(const char* i_axisName, std::map<std::string, KeyCodes::eKeyCodes*> i_axisKeyMap);

		/** Execute the binding functions*/
		void Execute(float i_axisValue) { m_boundDelegate->Execute(i_axisValue); }

		/** Setters */
		void SetDelegate(IDelegateHandler<float>* i_boundDelegate) { m_boundDelegate = i_boundDelegate; }
		/** Getters */
		const KeyCodes::eKeyCodes* GetBoundKey() const { return m_boundKeyCode; }

	private:
		KeyCodes::eKeyCodes m_boundKeyCode[2];
		// Delegate with one float parameter
		IDelegateHandler<float>* m_boundDelegate;
	};

	// -------------------------------
	// Advanced UserInput
	// Advanced UserInput struct helps user use input tools
	struct AdvancedUserInput
	{
	public:
		// --------------------
		// Assets
		using Handle = Assets::cHandle<AdvancedUserInput>;
		static Assets::cAssetManager<AdvancedUserInput> s_manager;
		// Initialize Data functions
		/** Factory Function to create advanced user input, it should load all action axis maps*/
		static bool Load(const std::string i_path, AdvancedUserInput*& o_ptr);
		bool ReadMappings(std::map<const char*, KeyCodes::eKeyCodes>, std::map<const char*, FAxisBrother>);
		static bool RemoveUserInput(AdvancedUserInput*& o_ptr);
		static bool LoadFileFromLUA(const std::string i_path, std::map<const char*, KeyCodes::eKeyCodes>& i_axisMapping, std::map<const char*, FAxisBrother>& i_actionMapping);
		// ---------------------
		/** Core update function to check and update binding maps. Should be called in a Tick function*/
		void UpdateInput();

#pragma region BindAction
		/** Bind an action to a [void] static / global function with no input parameter
		* i_actionName: the name of the action in data table
		* i_inputType: the input event
		* i_func: function to bound
		*/
		bool BindAction(const char* i_actionName, const InputType i_inputType, void(*i_func)())
		{
			bool result = true;
			if (!(result = IsActionNameValid(i_actionName))) {
				// Invalid action name
				printf("The action name: [%s] doesn't exist in the data\n", i_actionName);
				assert(result);
				return result;
			}
			ActionBinding newActionBinding = ActionBinding(i_actionName, m_actionKeyMap, i_inputType);
			IDelegateHandler<>* boundDelegate = StaticFuncDelegate<>::CreateStaticDelegate(i_func);
			newActionBinding.SetDelegate(boundDelegate);
			m_actionBindings.push_back(newActionBinding);
			return result;
		}

		/** Bind an action to a [void] member function with no input parameter
		* i_actionName: the name of the action in data table
		* i_inputType: the input event
		* i_ptr: the instance of the template class
		* i_func: function to bound
		*/
		template<class T>
		bool BindAction(const char* i_actionName, const InputType i_inputType, T* i_ptr, void(T::* i_func)())
		{
			bool result = Results::Success;
			if (!(result = IsActionNameValid(i_actionName))) {
				printf("The action name: [%s] doesn't exist in the data\n", i_actionName);
				assert(result);
				return result;
			}
			ActionBinding newActionBinding = ActionBinding(i_actionName, m_actionKeyMap, i_inputType);
			IDelegateHandler<>* boundDelegate = MemberFuncDelegate<T>::CreateMemberDelegate(i_ptr, i_func);
			newActionBinding.SetDelegate(boundDelegate);
			m_actionBindings.push_back(newActionBinding);
			return result;
		}

#pragma endregion

#pragma region BindAxis
		/** Bind an action to a [void] static / global function with a float parameter
		* i_axisName: the name of the axis in data table
		* i_func: function to bound, the input parameter should be a float
		*/
		bool BindAxis(const char* i_axisName, void(*i_func)(float)) {
			bool result = true;
			if (!(result = IsAxisNameValid(i_axisName))) {
				// Invalid axis name
				printf("The axis name: [%s] doesn't exist in the data\n", i_axisName);
				assert(result);
				return result;
			}
			AxisBinding newAxisBinding = AxisBinding(i_axisName, m_axisKeyMap);
			IDelegateHandler<float>* boundDelegate = StaticFuncDelegate<float>::CreateStaticDelegate(i_func);
			newAxisBinding.SetDelegate(boundDelegate);
			m_axisBindings.push_back(newAxisBinding);
			return result;
		}

		/** Bind an action to a [void] member function with with a float parameter
		* i_actionName: the name of the axis in data table
		* i_ptr: the instance of the template class
		* i_func: function to bound, the input parameter should be a float
		*/
		template<class T>
		bool BindAxis(const char* i_axisName, T* i_ptr, void(T::* i_func)(float))
		{
			bool result = Results::Success;
			if (!(result = IsAxisNameValid(i_axisName))) {
				// Invalid axis name
				printf("The axis name: [%s] doesn't exist in the data\n", i_axisName);
				assert(result);
				return result;
			}
			AxisBinding newAxisBinding = AxisBinding(i_axisName, m_axisKeyMap);
			IDelegateHandler<float>* boundDelegate = MemberFuncDelegate<T, float>::CreateMemberDelegate(i_ptr, i_func);
			newAxisBinding.SetDelegate(boundDelegate);
			m_axisBindings.push_back(newAxisBinding);
			return result;
		}
#pragma endregion

		// ---------------------
		// Global checker functions
		/** Check if this axis is pressed*/
		bool IsActionPressed(const char* i_actionName);

		/** Check if this axis is released*/
		bool IsActionReleased(const char* i_actionName);


	private:
		// ---------------------
		// private Initialize function
		AdvancedUserInput() : m_actionBindings(), m_actionKeyMap(), m_axisBindings(), m_axisKeyMap()
		{}
		~AdvancedUserInput();

		// ---------------------
		// Action bindings
		/** List of action bindings*/
		std::vector<ActionBinding> m_actionBindings;
		/** Action Name to keyCode map*/
		std::map<std::string, KeyCodes::eKeyCodes> m_actionKeyMap;

		// ---------------------
		// Axis bindings
		/** List of action bindings*/
		std::vector<AxisBinding> m_axisBindings;
		/** Axis Name to keyCodes map*/
		std::map<std::string, KeyCodes::eKeyCodes*> m_axisKeyMap;

		// ---------------------
		// Helper utilities
		/** Check if key down*/
		bool IsKeyDown(const KeyCodes::eKeyCodes& i_key) const;

		/** Check if key codes already exist in key status map */
		bool IsKeyExisted(const KeyCodes::eKeyCodes& i_key) const;

		/** Check if the input action name is a valid axis*/
		bool IsActionNameValid(const char* i_actionName);

		/** Check if the input axis name is a valid axis*/
		bool IsAxisNameValid(const char* i_axisName);

		/** Add action key pair to map*/
		void AddActionKeyPairToMap(const char* i_actionName, const KeyCodes::eKeyCodes& i_keycode);

		/** Add action key pair to map*/
		void AddAxisKeyPairToMap(const char* i_axisName, KeyCodes::eKeyCodes* i_keycodes);

		/** record the outdated key code status map, true -> keyDown*/
		std::map< KeyCodes::eKeyCodes, bool> m_keyStatusMap;
	};

}