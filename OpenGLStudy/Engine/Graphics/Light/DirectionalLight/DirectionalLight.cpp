#include "DirectionalLight.h"
namespace Graphics {

	cDirectionalLight::cDirectionalLight() : m_directionID(0), m_direction(glm::vec3(0, -1, 0)), cGenLight()
	{}

	cDirectionalLight::~cDirectionalLight()
	{
	}


	void cDirectionalLight::SetupLight(GLfloat i_intensity, glm::vec3 i_color, const GLuint& i_programID)
	{
		cGenLight::SetupLight(i_intensity, i_color, i_programID);

		m_colorID = glGetUniformLocation(i_programID, "directionalLight.color");
		m_intensityID = glGetUniformLocation(i_programID, "directionalLight.intensity");
		m_directionID = glGetUniformLocation(i_programID, "directionalLight.direction");
	}

	void cDirectionalLight::SetupLightDirection(glm::vec3 i_direction)
	{
		m_direction = glm::normalize(i_direction);
	}

	void cDirectionalLight::Illuminate()
	{
		glUniform3f(m_colorID, m_color.x, m_color.y, m_color.z);
		glUniform1f(m_intensityID, m_intensity);
		glUniform3f(m_directionID, m_direction.x, m_direction.y, m_direction.z);
	}

	glm::vec3 cDirectionalLight::Direction(glm::vec3 i_position)
	{
		return glm::vec3(0, 0, 0);
	}

}