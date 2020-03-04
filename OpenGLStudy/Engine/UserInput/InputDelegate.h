#pragma once
#include <typeinfo>

namespace UserInput {
	// ---------------------
	/** interface for the general functions of delegate*/
	template<typename ...ParamType>
	class IDelegateHandler
	{
	public:
		IDelegateHandler() {};
		virtual ~IDelegateHandler() {};

		/** if the type is the input type*/
		virtual bool IsType(const std::type_info& i_type) const = 0;
		/** Execute the function or functions*/
		virtual void Execute(ParamType ...params) = 0;
		/** Check If two handlers are the same one, basically doing favor for multi-cast */
		virtual bool Compare(IDelegateHandler* i_handler) const = 0;
	};

	// -------------------
	/** Static function delegate with zero or multiple input parameter, return void*/
	template<typename ...ParamType>
	class StaticFuncDelegate : public IDelegateHandler<ParamType ...>
	{
	public:
		/** Typedef a Func as a void delegate function with more than one parameter*/
		typedef void(*Func)(ParamType...);

		//** Function to create static function delegate*/
		template< typename T>
		static IDelegateHandler<ParamType...>* CreateStaticDelegate(T i_func)
		{
			return new StaticFuncDelegate<ParamType...>(i_func);
		}

		/** Constructor for Static function delegate*/
		StaticFuncDelegate(Func i_func) : m_Func(i_func) { }

		virtual bool IsType(const std::type_info& i_type) const override { return typeid(StaticFuncDelegate<ParamType ...>) == i_type; }

		virtual void Execute(ParamType... params) override { m_Func(params...); }

		virtual bool Compare(IDelegateHandler<ParamType ...>* i_handler) const override
		{
			if (0 == i_handler || !i_handler->IsType(typeid(StaticFuncDelegate<ParamType ...>))) return false;
			StaticFuncDelegate<ParamType ...>* cast = static_cast<StaticFuncDelegate<ParamType ...>*>(i_handler);
			return cast->m_Func == m_Func;
		}

	private:
		Func m_Func;
	};

	// -------------------
	/** Member function delegate with zero or multiple input parameter, return void*/
	template<typename T, typename ...ParamType>
	class MemberFuncDelegate : public IDelegateHandler<ParamType ...>
	{
	public:
		/** Typedef a Func as a member void delegate function with more than one parameter*/
		typedef void (T::*Func)(ParamType...);

		//** Function to create member function delegate*/
		template<typename T, typename F>
		static IDelegateHandler<ParamType...>* CreateMemberDelegate(T* i_ptr, F i_func) {
			return new MemberFuncDelegate<T, ParamType...>(i_ptr, i_func);
		}

		/** Constructor for Member function delegate*/
		MemberFuncDelegate(T* i_ptr, Func i_func) : m_ptr(i_ptr), m_Func(i_func) {}

		virtual bool IsType(const std::type_info& i_type) const override { return typeid(MemberFuncDelegate<T, ParamType ...>) == i_type; }

		virtual void Execute(ParamType... params) override { (m_ptr->*m_Func)(params...); }

		virtual bool Compare(IDelegateHandler<ParamType ...>* i_handler) const override
		{
			if (0 == i_handler || !i_handler->IsType(typeid(MemberFuncDelegate<T, ParamType ...>))) return false;
			MemberFuncDelegate<T, ParamType ...>* cast = static_cast<MemberFuncDelegate<T, ParamType ...>*>(i_handler);
			return cast->m_Func == m_Func && cast->m_ptr == m_ptr;
		}

	private:
		T* m_ptr;
		Func m_Func;
	};
}


