#pragma once
#include <thread>
#include <mutex>
#include <typeinfo>

#include "Engine/Cores/Core.h"
#include "gl/glew.h"
#include "glfw/glfw3.h"
/** Forward deceleration*/
//----------------------------------------------
class cWindow;
//----------------------------------------------
namespace Application {

	class cApplication
	{
	public:
		cApplication() {}
		~cApplication() { CleanUp(); };

		virtual bool Initialize(GLuint i_width, GLuint i_height, const char* i_windowName = "Default Window");
		virtual bool PostInitialization();
		virtual void CleanUp();
		virtual void Render() {};

		// Real time update, call as much as possible
		virtual void Tick(float second_since_lastFrame) {}
		// Simulation time update, call in fixed rate, set by m_simulationUpdateRate_InSeconds
		virtual void FixedTick(){}
		
		void UpdateUntilExit();

		cWindow* GetCurrentWindow() const { return m_window; }

	protected:
		cApplication(const cApplication& i_other) = delete;
		cApplication& operator = (const cApplication& i_other) = delete;

		/** Handle timing*/
		//---------------------------------------------------
		bool m_shouldApplicationLoopExit = false;
		uint64_t m_tickCount_systemTime_WhenApplicationStart = false;
		uint64_t m_tickCount_systemTime_Current = 0;
		uint64_t m_tickCountt_systemTime_Elapsed = 0;
		// Fixed update rate, in default case: function should be called every 0.016667f seconds, which is every 1/0.016667f = 60 ticks
		float m_simulationUpdateRate_InSeconds = 0.016667f;

		/** Handle threading*/
		//---------------------------------------------------
		std::mutex m_applicationMutex;
		std::thread* m_applicationThread;
		// Start an application thread
		void ApplicationLoopThread(void* const io_application);


		/** protected variables */
		//---------------------------------------------------
		//Current window 
		cWindow* m_window;
	};

	extern cApplication* s_currentApplication;

	// global functions
	template<class APP >
	bool CreateApplication(APP*& o_app, GLuint i_width, GLuint i_height, const char* i_windowName/* = "Default Window"*/)
	{
		auto result = true;

		cApplication* _newApp = new APP();
		s_currentApplication = reinterpret_cast<APP*>(_newApp);
		if (!(result = _newApp->Initialize(i_width, i_height, i_windowName)))
		{
			printf("Failed to create application!");
			safe_delete(_newApp);
			return result;
		}
		o_app = reinterpret_cast<APP*>(_newApp);
		return result;
	}

	template<class APP>
	void DestroyApplication(APP*& io_app)
	{
		cApplication* _tempApp;
		if (_tempApp = reinterpret_cast<cApplication*>(io_app)) 
		{
			_tempApp->CleanUp();
			safe_delete(io_app);
		}
		else
		{
			printf("This is not an application!\n");
		}
	}

	cApplication* GetCurrentApplication();

}
