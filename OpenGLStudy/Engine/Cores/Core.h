#pragma once
#ifndef CORE_DEFINED
#define CORE_DEFINED


#define safe_delete(x) if(x){delete x; x = nullptr;}
#define MAX_COUNT_PER_LIGHT 5
#define OMNI_SHADOW_MAP_COUNT 5
#define SHADOWMAP_START_TEXTURE_UNIT 14
#define IBL_CUBEMAP_START_TEXTURE_UNIT 6

#define IsFloatZero(x) (x > -0.0001f && x < 0.0001f)

/** Check memory leak*/
#ifdef _DEBUG
//#define NEED_CHECK_MEMORY_LEAK
#ifdef NEED_CHECK_MEMORY_LEAK
#include <crtdbg.h>
#define DEBUG_NEW_PLACEMENT (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_NEW_PLACEMENT
#endif
#endif // NEED_CHECK_MEMORY_LEAK

#endif