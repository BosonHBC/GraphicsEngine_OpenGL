#include "Graphics/Light/Light.h"

namespace Graphics {

	cGenLight::~cGenLight()
	{
	}

	void cGenLight::SetupLight(GLfloat i_intensity, glm::vec3 i_color, const GLuint& i_programID)
	{
		m_color = i_color;
		m_intensity = i_intensity;
	}

}
