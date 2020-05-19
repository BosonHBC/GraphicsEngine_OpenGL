#pragma once
#include <cstdint>
namespace Profiler
{
	enum eProfileType : uint8_t
	{
		EPT_RenderAFrame,
		EPT_GBuffer,
		EPT_PointLightShadowMap,
		EPT_DeferredLighting,
		EPT_Selection,
		EPT_Invalid = -1,
	};

	struct sProfilerUnit
	{
		eProfileType ProfileType;
		unsigned int queryID[2];

		sProfilerUnit(eProfileType i_type) : ProfileType(i_type){}
		sProfilerUnit() : ProfileType(eProfileType::EPT_Invalid) { queryID[0] = 0; queryID[1] = 0; }

	};

	bool CreateProfiler(eProfileType i_type);
	bool StartRecording(eProfileType i_type);
	bool StopRecording(eProfileType i_type);

	bool GetProfilingTime(eProfileType i_type, float& o_time);
	
	void CleanUp();
}

