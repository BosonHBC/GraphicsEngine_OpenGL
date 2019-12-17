#include "Time.h"
#include "stdio.h"
#include <assert.h>
#include "Windows.h"
namespace {
	uint64_t s_tickCountPerSecond = 0;
}

namespace Time {
	
	bool Initialize()
	{
		LARGE_INTEGER tickCountPerSecond;
		if (QueryPerformanceFrequency(&tickCountPerSecond) != FALSE)
		{
			if (tickCountPerSecond.QuadPart != 0)
			{
				s_tickCountPerSecond = static_cast<uint64_t>(tickCountPerSecond.QuadPart);
			}
			else
			{
				printf("This hardware doesn't support high resolution performance counters!");
				return false;
			}
		}
		else
		{
			assert(false && "Windows failed to query performance frequency");
			return false;
		}
	}

	uint64_t GetCurrentSystemTimeTickCount()
	{
		LARGE_INTEGER totalTickCountSinceSystemBoot;
		const auto result = QueryPerformanceCounter(&totalTickCountSinceSystemBoot);
		// Microsoft claims that querying the counter will never fail on Windows XP or later:
		// https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx
		assert(result != FALSE && "QueryPerformanceCounter() failed");
		return static_cast<uint64_t>(totalTickCountSinceSystemBoot.QuadPart);
	}

	GLfloat ConvertFromTickToSeconds(const uint64_t& i_tickCount)
	{
		assert(s_tickCountPerSecond > 0 && "s_tickCountPerSecond is smaller or equal to zero");
		return static_cast<double>(i_tickCount) / static_cast<double>(s_tickCountPerSecond);
	}

}