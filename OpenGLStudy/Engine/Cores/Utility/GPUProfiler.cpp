#include "GPUProfiler.h"
#include <unordered_map>

#include "GL/glew.h"
#include "glfw/glfw3.h"
namespace Profiler
{
	std::unordered_map<eProfileType, sProfilerUnit> g_profileUnits;

	bool CreateProfiler(eProfileType i_type)
	{
		if (g_profileUnits.find(i_type) != g_profileUnits.end()) return false;
		g_profileUnits.insert({ i_type, sProfilerUnit(i_type) });

		glGenQueries(2, g_profileUnits[i_type].queryID);
		return true;
	}

	bool StartRecording(eProfileType i_type)
	{
		if (g_profileUnits.find(i_type) == g_profileUnits.end()) return false;
		const sProfilerUnit& _unit = g_profileUnits.at(i_type);

		glQueryCounter(_unit.queryID[0], GL_TIMESTAMP);
	}

	bool StopRecording(eProfileType i_type)
	{
		if (g_profileUnits.find(i_type) == g_profileUnits.end()) return false;
		const sProfilerUnit& _unit = g_profileUnits.at(i_type);

		glQueryCounter(_unit.queryID[1], GL_TIMESTAMP);
	}

	bool GetProfilingTime(eProfileType i_type, float& o_time)
	{
		if (g_profileUnits.find(i_type) == g_profileUnits.end()) return false;

		const sProfilerUnit& _unit = g_profileUnits.at(i_type);

/*
		GLint stopTimerValid = 0;
		int deadEndPreventCount = 1000;
		while (!stopTimerValid && deadEndPreventCount-- > 0)
		{
			glGetQueryObjectiv(_unit.queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerValid);
		}
		if (deadEndPreventCount <= 0) return false;*/
		
		GLuint64 startTime = 0, stopTime= 0;
		glGetQueryObjectui64v(_unit.queryID[0], GL_QUERY_RESULT, &startTime);
		glGetQueryObjectui64v(_unit.queryID[1], GL_QUERY_RESULT, &stopTime);

		o_time = (stopTime - startTime) / 1000000.f ;
		return true;
	}

	void CleanUp()
	{
		for(auto item : g_profileUnits)
		{
			glDeleteQueries(2, item.second.queryID);
		}
		g_profileUnits.clear();
	}

}