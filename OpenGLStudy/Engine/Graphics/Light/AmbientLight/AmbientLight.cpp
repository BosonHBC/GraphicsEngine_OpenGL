#include "AmbientLight.h"
namespace Graphics {

	cAmbientLight::~cAmbientLight()
	{
	}


	void cAmbientLight::SetupLight(GLfloat i_intensity, glm::vec3 i_color, const GLuint& i_programID)
	{
		cGenLight::SetupLight(i_intensity, i_color, i_programID);

		m_ambientColorID = glGetUniformLocation(i_programID, "ambientLight.color");
		m_ambientIntensityID = glGetUniformLocation(i_programID, "ambientLight.intensity");
	}

	void cAmbientLight::Illuminate()
	{
		glUniform3f(m_ambientColorID, m_color.x, m_color.y, m_color.z);
		glUniform1f(m_ambientIntensityID, m_intensity);
	}


}