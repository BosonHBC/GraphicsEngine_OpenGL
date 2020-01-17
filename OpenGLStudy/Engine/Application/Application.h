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
		virtual void UpdateBasedOnTime(float second_since_lastFrame) {}
		
		void UpdateUntilExit();

		cWindow* Get_GLFW_Window() const { return m_window; }
	protected:
		cWindow* m_window;

		uint64_t m_tickCount_systemTime_WhenApplicationStart = false;
		uint64_t m_tickCount_systemTime_Current = 0;
		uint64_t m_tickCountt_systemTime_Elapsed = 0;
		bool m_shouldApplicationLoopExit = false;
		std::thread* m_applicationThread;

		void ApplicationLoopThread(void* const io_application);

	};


}
