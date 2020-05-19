/** This time manage the time-related functionality

	CREDITS TO: JOHN PAUL, professor of EAE 6320, University of Utah,
	Implementations are inspired by JOHN PAUL's engine. I have made some changes to fulfill my requirement.
*/
#pragma once

#include "gl/glew.h"
#include "glfw/glfw3.h"

namespace Time {

	bool Initialize();

	uint64_t GetCurrentSystemTimeTickCount();
	double ConvertFromTickToSeconds(const uint64_t& i_tickCount);
	uint64_t ConvertFromSecondsToTick(const double& i_second);
	float DeltaTime();
	void SetDeltaTime(uint64_t tickCount_systemTime_elapsedSinceLastLoop);
}
