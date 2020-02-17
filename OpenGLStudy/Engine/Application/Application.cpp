#include <algorithm>
#include "assert.h"
#include "Application.h"
#include "Engine/Application/Window/Window.h"
#include "Time/Time.h"
#include "Graphics/Graphics.h"

namespace Application {


	bool cApplication::Initialize(GLuint i_width, GLuint i_height, const char* i_windowName)
	{
		bool result = true;
		// Initialize Time
		if (result = Time::Initialize())
		{
			m_tickCount_systemTime_WhenApplicationStart = Time::GetCurrentSystemTimeTickCount();
		}
		else {
			printf("Application can't be initialized without Time\n");
			return result;
		}

		// Create a GLFW window
		{
			m_window = new cWindow(i_width, i_height, i_windowName);
			if (!(result = m_window->Initialzation())) {
				printf("Failed to initialize openGL window!\n");
				return result;
			}
		}

		if (!(result = Graphics::Initialize())) {
			printf("Failed to initialize Graphics module!\n");
			return result;
		}

		return result;
	}

	bool cApplication::PostInitialization()
	{
		auto result = true;

		// Create application thread by lambda
		m_applicationThread = new std::thread(
			[&](cApplication* app)
			{
				app->UpdateUntilExit();
			}
		, this);
		return result;
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
		const auto tickCount_per_simulationUpdate = Time::ConvertFromSecondsToTick(m_simulationUpdateRate_InSeconds);
		auto tickCount_ElapsedBetween_twoSimulationUpdate = uint64_t(0);
		// In debug mode, application may freeze but the tick will not.
		// To prevent the second_since_last_update from being too large, need to add a max limitation for it. 
		const auto tickCount_maxAllowable_time_per_Iteration = Time::ConvertFromSecondsToTick(0.5);

		// Update until application thread exits
		while (!m_shouldApplicationLoopExit)
		{
			// Calculate how much time has elapsed since the last loop
			uint64_t tickCount_systemTime_elapsedSinceLastLoop;
			// Update based on time
			{
				// previous
				const auto tickCount_systemTime_previousLoop = tickCount_systemTime_currentLoop;
				// current
				tickCount_systemTime_currentLoop = Time::GetCurrentSystemTimeTickCount();
				// record current tick count
				m_tickCount_systemTime_Current = tickCount_systemTime_currentLoop;

				// calculate the delta tick between loop, make sure the update between will not be larger than the maxAllowable_time_per_Iteration
				tickCount_systemTime_elapsedSinceLastLoop =
					std::min(tickCount_systemTime_currentLoop - tickCount_systemTime_previousLoop, tickCount_maxAllowable_time_per_Iteration);

				// Update as soon as possible
				Tick(static_cast<float>(Time::ConvertFromTickToSeconds(tickCount_systemTime_elapsedSinceLastLoop)));

				m_tickCountt_systemTime_Elapsed += tickCount_systemTime_elapsedSinceLastLoop;
			}

			// Update simulation (fixed update)
			{
				tickCount_ElapsedBetween_twoSimulationUpdate += tickCount_systemTime_elapsedSinceLastLoop;

				// When the tick count elapse between two simulation is bigger than the interval, call update simulation
				if (tickCount_ElapsedBetween_twoSimulationUpdate >= tickCount_per_simulationUpdate) {
					FixedTick();
					tickCount_ElapsedBetween_twoSimulationUpdate -= tickCount_per_simulationUpdate;
				}
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
		if (!Graphics::CleanUp()) {
			printf("Fail to clean up Graphic Module.\n");
		}
		safe_delete(m_window);
	}

	
	cApplication* s_currentApplication = nullptr;


	cApplication* GetCurrentApplication()
	{
		return s_currentApplication;
	}

}
