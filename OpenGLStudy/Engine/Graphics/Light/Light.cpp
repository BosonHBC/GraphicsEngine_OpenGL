#include "Graphics/Light/Light.h"
#define MAX_COUNT_PER_LIGHT 3
namespace Graphics {

	cGenLight::~cGenLight()
	{
	}

	void cGenLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		m_lightIndex = (i_lightIndex < MAX_COUNT_PER_LIGHT)? i_lightIndex : MAX_COUNT_PER_LIGHT-1;
	}

}
