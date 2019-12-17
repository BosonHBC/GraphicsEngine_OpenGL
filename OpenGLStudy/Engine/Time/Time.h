#pragma once

#include "gl/glew.h"
#include "glfw/glfw3.h"

/** This time manage the time-related functionality
	The idea comes from eae6320 class from University of Utah
	Some of the code is referencing our professor John-Paul' s code he used during class
*/
namespace Time {

	bool Initialize();

	uint64_t GetCurrentSystemTimeTickCount();
	GLfloat ConvertFromTickToSeconds(const uint64_t& i_tickCount);

}
