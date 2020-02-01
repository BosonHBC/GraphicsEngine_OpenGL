#pragma once
class cWindow;

#include "gl/glew.h"
#include "glfw/glfw3.h"
#include <thread>

namespace Application {
	class cApplication
	{
	public:
		cApplication();
		virtual ~cApplication();

		virtual bool Initialize(GLuint i_width, GLuint i_height, const char* i_windowName = "Default Window");
		virtual void PostInitialization();
		virtual void CleanUp();
		virtual void Run() {};

		// Real time update, call as much as possible
		virtual void Tick(float second_since_lastFrame) {}
		// Simulation time update, call in fixed rate, set by m_simulationUpdateRate_InSeconds
		virtual void FixedTick(){}
		
		void UpdateUntilExit();

		cWindow* Get_GLFW_Window() const { return m_window; }

	protected:
		cWindow* m_window;

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
		std::thread* m_applicationThread;
		void ApplicationLoopThread(void* const io_application);

	};


}
