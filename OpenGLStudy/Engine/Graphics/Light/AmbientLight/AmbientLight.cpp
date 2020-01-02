#include "AmbientLight.h"
namespace Graphics {

	cAmbientLight::cAmbientLight(): cGenLight()
	{

	}

	cAmbientLight::~cAmbientLight()
	{
	}


	void cAmbientLight::SetupLight(GLfloat i_intensity, glm::vec3 i_color, const GLuint& i_programID)
	{
		cGenLight::SetupLight(i_intensity, i_color, i_programID);

		m_colorID = glGetUniformLocation(i_programID, "ambientLight.color");
		m_intensityID = glGetUniformLocation(i_programID, "ambientLight.intensity");
	}

	void cAmbientLight::Illuminate()
	{
		glUniform3f(m_colorID, m_color.x, m_color.y, m_color.z);
		glUniform1f(m_intensityID, m_intensity);
	}


}