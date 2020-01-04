#include "AmbientLight.h"
namespace Graphics {

	cAmbientLight::~cAmbientLight()
	{
	}


	void cAmbientLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight( i_programID, i_lightIndex);

		m_colorID = glGetUniformLocation(i_programID, "ambientLight.base.color");

		m_diffuseIntensityID = glGetUniformLocation(i_programID, "ambientLight.base.diffuseIntensity");
		m_specularIntensityID = glGetUniformLocation(i_programID, "ambientLight.base.specularIntensity");
	}

	void cAmbientLight::Illuminate()
	{
		glUniform3f(m_colorID, m_color.x, m_color.y, m_color.z);
		glUniform1f(m_diffuseIntensityID, m_diffuseIntensity);
		glUniform1f(m_specularIntensityID, m_specularIntensity);
	}


}