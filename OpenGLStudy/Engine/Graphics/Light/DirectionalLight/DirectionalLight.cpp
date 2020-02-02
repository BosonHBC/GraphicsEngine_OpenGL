#include "DirectionalLight.h"
namespace Graphics {

	cDirectionalLight::~cDirectionalLight()
	{
	}


	void cDirectionalLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

		m_colorID = glGetUniformLocation(i_programID, "directionalLight.base.color");
		m_directionID = glGetUniformLocation(i_programID, "directionalLight.direction");
	}

	void cDirectionalLight::Illuminate()
	{
		glUniform3f(m_colorID, m_color.r, m_color.g, m_color.b);
		glUniform3f(m_directionID, m_direction.x, m_direction.y, m_direction.z);
	}

	glm::vec3 cDirectionalLight::Direction(glm::vec3 i_position)
	{
		return m_direction;
	}

}