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
		CleanUp();
	}

	bool cApplication::Initialize(GLuint i_width, GLuint i_height)
	{
		bool result = true;
		// Initialize Time
		if (result = Time::Initialize())
		{
			m_tickCount_systemTime_WhenApplicationStart = Time::GetCurrentSystemTimeTickCount();
		}
		else {
			assert(false && "Application can't be initialized without Time");
			return result;
		}

		// Create a GLFW window
		{
			m_window = new cWindow(i_width, i_height);
			if (!m_window->Initialzation()) {
				printf("Failed to initialize openGL window!");
				return false;
			}
		}

		// Create application thread by lambda
		m_applicationThread = new std::thread(
			[&](cApplication* app)
			{
				app->UpdateUntilExit();
			}
		, this);
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
			// Update by delta seconds
			{
				UpdateBasedOnTime(static_cast<float>(Time::ConvertFromTickToSeconds(tickCount_systemTime_elapsedSinceLastLoop)));
			}
			m_shouldApplicationLoopExit = m_window->GetShouldClose();
		}

	}

	void cApplication::CleanUp()
	{
		if (m_applicationThread) {
			m_applicationThread->join();
			delete m_applicationThread;
			m_applicationThread = nullptr;
		}
		if (m_window) {
			delete m_window;
			m_window = nullptr;
		}
	}

}
