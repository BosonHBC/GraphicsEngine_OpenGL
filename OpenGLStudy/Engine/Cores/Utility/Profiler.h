#pragma once
#include <cstdint>
namespace Profiler
{
	// GPU profiling
	enum eGPUProfileType : uint8_t
	{
		EPT_RenderAFrame,
		EPT_GBuffer,
		EPT_PointLightShadowMap,
		EPT_DeferredLighting,
		EPT_Selection,
		EPT_Invalid = -1,
	};

	struct sGPUProfilerUnit
	{
		eGPUProfileType ProfileType;
		unsigned int queryID[2];

		sGPUProfilerUnit(eGPUProfileType i_type) : ProfileType(i_type){}
		sGPUProfilerUnit() : ProfileType(eGPUProfileType::EPT_Invalid) { queryID[0] = 0; queryID[1] = 0; }

	};

	struct sCPUProfileUnit
	{
		uint64_t startTickCount;
		uint64_t endTickCount;

		float GetTimeInms();
	};

	bool CreateProfiler(eGPUProfileType i_type);
	bool StartRecording(eGPUProfileType i_type);
	bool StopRecording(eGPUProfileType i_type);
	bool GetProfilingTime(eGPUProfileType i_type, float& o_time);
	
	// CPU profiling
	void StartRecording(int i_idx);
	void StopRecording(int i_idx);
	bool GetProfilingTime(int i_idx, float& o_time);

	void CleanUp();
}

