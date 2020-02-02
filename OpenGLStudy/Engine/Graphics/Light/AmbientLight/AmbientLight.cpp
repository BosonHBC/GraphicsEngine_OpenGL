#include "AmbientLight.h"
namespace Graphics {

	cAmbientLight::~cAmbientLight()
	{
	}


	void cAmbientLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight( i_programID, i_lightIndex);

		m_colorID = glGetUniformLocation(i_programID, "ambientLight.base.color");

	}

	void cAmbientLight::Illuminate()
	{
		glUniform3f(m_colorID, m_color.r, m_color.g, m_color.b);
	}


}