#include "stdio.h"
#include "assert.h"
#include "Application.h"
#include "Graphics/Window/Window.h"
#include "Time/Time.h"

namespace Application {
	cApplication::cApplication()
	{
	}

	cApplication::~cApplication()
	{
		if (m_window) {
			delete m_window;
			m_window =  nullptr;
		}
	}

	bool cApplication::Initialize(GLuint i_width, GLuint i_height)
	{
		bool result = true;
		// Initialize Time
		if(result = Time::Initialize())
		{
			m_tickCount_systemTime_WhenApplicationStart = Time::GetCurrentSystemTimeTickCount();
		}
		else {
			assert(false && "Application can't be initialized without Time");
			return result;
		}

		// Create a GLFW window
		{
			cWindow* newWindow = new cWindow(i_width, i_height);
			if (!newWindow->Initialzation()) {
				printf("Failed to initialize openGL window!");
				return false;
			}
		}

		Run();
		m_applicationThread = new std::thread([&](cApplication* app) {
			app->UpdateUntilExit();
			}, this);
		return true;
	}

	void cApplication::ApplicationLoopThread(void* const io_application)
	{
		cApplication* application = static_cast<cApplication*>(io_application);
		assert(application);
		return application->UpdateUntilExit();
	}

	void cApplication::UpdateUntilExit()
	{
		auto tickCount_systemTime_currentLoop = Time::GetCurrentSystemTimeTickCount();
		m_tickCount_systemTime_Current = tickCount_systemTime_currentLoop;

		while (!m_shouldApplicationLoopExit)
		{
			// Calculate how much time has elapsed since the last loop
			uint64_t tickCount_systemTime_elapsedSinceLastLoop;
			{

				const auto tickCount_systemTime_previousLoop = tickCount_systemTime_currentLoop;
				tickCount_systemTime_currentLoop = Time::GetCurrentSystemTimeTickCount();
				m_tickCount_systemTime_Current = tickCount_systemTime_currentLoop;

				tickCount_systemTime_elapsedSinceLastLoop = tickCount_systemTime_currentLoop - tickCount_systemTime_previousLoop;
			}
			// Update any application state that isn't part of the simulation
			{
				UpdateBasedOnTime(static_cast<float>(Time::ConvertFromTickToSeconds(tickCount_systemTime_elapsedSinceLastLoop)));
			}
			m_shouldApplicationLoopExit = m_window->GetShouldClose();
		}
	}

	void cApplication::CleanUp()
	{
		if (m_applicationThread) {
			delete m_applicationThread;
			m_applicationThread = nullptr;
		}
	}

}
