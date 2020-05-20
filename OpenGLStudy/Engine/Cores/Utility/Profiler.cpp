#include "Profiler.h"
#include <unordered_map>

#include "GL/glew.h"
#include "glfw/glfw3.h"
#include "Time/Time.h"
namespace Profiler
{
	std::unordered_map<eGPUProfileType, sGPUProfilerUnit> g_GPUProfileUnits;
	std::unordered_map<int, sCPUProfileUnit> g_CPUProfileUnits;

	bool CreateProfiler(eGPUProfileType i_type)
	{
		if (g_GPUProfileUnits.find(i_type) != g_GPUProfileUnits.end()) return false;
		g_GPUProfileUnits.insert({ i_type, sGPUProfilerUnit(i_type) });

		glGenQueries(2, g_GPUProfileUnits[i_type].queryID);
		return true;
	}

	bool StartRecording(eGPUProfileType i_type)
	{
		if (g_GPUProfileUnits.find(i_type) == g_GPUProfileUnits.end()) return false;
		const sGPUProfilerUnit& _unit = g_GPUProfileUnits.at(i_type);

		glQueryCounter(_unit.queryID[0], GL_TIMESTAMP);
	}

	bool StopRecording(eGPUProfileType i_type)
	{
		if (g_GPUProfileUnits.find(i_type) == g_GPUProfileUnits.end()) return false;
		const sGPUProfilerUnit& _unit = g_GPUProfileUnits.at(i_type);

		glQueryCounter(_unit.queryID[1], GL_TIMESTAMP);
	}



	bool GetProfilingTime(eGPUProfileType i_type, float& o_time)
	{
		if (g_GPUProfileUnits.find(i_type) == g_GPUProfileUnits.end()) return false;

		const sGPUProfilerUnit& _unit = g_GPUProfileUnits.at(i_type);

		/*
				GLint stopTimerValid = 0;
				int deadEndPreventCount = 1000;
				while (!stopTimerValid && deadEndPreventCount-- > 0)
				{
					glGetQueryObjectiv(_unit.queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerValid);
				}
				if (deadEndPreventCount <= 0) return false;*/

		GLuint64 startTime = 0, stopTime = 0;
		glGetQueryObjectui64v(_unit.queryID[0], GL_QUERY_RESULT, &startTime);
		glGetQueryObjectui64v(_unit.queryID[1], GL_QUERY_RESULT, &stopTime);

		o_time = (stopTime - startTime) / 1000000.f;
		return true;
	}



	void StartRecording(int i_idx)
	{
		if (g_CPUProfileUnits.find(i_idx) == g_CPUProfileUnits.end())
			g_CPUProfileUnits.insert({ i_idx , sCPUProfileUnit() });
		else
		{
			sCPUProfileUnit& _unit = g_CPUProfileUnits.at(i_idx);
			_unit.startTickCount = Time::GetCurrentSystemTimeTickCount();
		}
	}

	void StopRecording(int i_idx)
	{
		if (g_CPUProfileUnits.find(i_idx) == g_CPUProfileUnits.end()) return;

		sCPUProfileUnit& _unit = g_CPUProfileUnits.at(i_idx);
		_unit.endTickCount = Time::GetCurrentSystemTimeTickCount();
	}

	bool GetProfilingTime(int i_idx, float& o_time)
	{
		if (g_CPUProfileUnits.find(i_idx) == g_CPUProfileUnits.end()) return false;
		sCPUProfileUnit& _unit = g_CPUProfileUnits.at(i_idx);
		o_time = _unit.GetTimeInms();
		return true;
	}

	void CleanUp()
	{
		for (auto item : g_GPUProfileUnits)
		{
			glDeleteQueries(2, item.second.queryID);
		}
		g_GPUProfileUnits.clear();
	}

	float sCPUProfileUnit::GetTimeInms()
	{
		return Time::ConvertFromTickToSeconds(endTickCount - startTickCount) * 1000;
	}

}