#include "PointLight.h"
#include <stdio.h>
namespace Graphics {


	cPointLight::cPointLight(const cPointLight& i_other)
		: cGenLight(i_other), m_const(i_other.m_const), m_linear(i_other.m_linear), m_quadratic(m_quadratic),
		m_positionID(i_other.m_positionID), m_constID(i_other.m_constID), m_linearID(i_other.m_linearID), m_quadraticID(i_other.m_quadraticID)
	{
	}

	cPointLight& cPointLight::operator=(const cPointLight& i_other)
	{
		cGenLight::operator=(i_other);
		m_const = i_other.m_const;
		m_linear = i_other.m_linear;
		m_quadratic = i_other.m_quadratic;
		m_positionID = i_other.m_positionID;
		m_constID = i_other.m_constID;
		m_linearID = i_other.m_linearID;
		m_quadraticID = i_other.m_quadraticID;
		return *this;
	}

	void cPointLight::Illuminate()
	{
		glUniform3f(m_colorID, m_color.r, m_color.g, m_color.b);
		glm::vec3 worldLoc = m_transform->GetWorldLocation();
		glUniform3f(m_positionID, worldLoc.x, worldLoc.y, worldLoc.z);
		glUniform1f(m_constID, m_const);
		glUniform1f(m_linearID, m_linear);
		glUniform1f(m_quadraticID, m_quadratic);
	}

	void cPointLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

		char _buff[100] = { '\0' };

		snprintf(_buff, sizeof(_buff), "pointLights[%d].base.color", m_lightIndex);
		m_colorID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].position", m_lightIndex);
		m_positionID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].constant", m_lightIndex);
		m_constID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].linear", m_lightIndex);
		m_linearID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].quadratic", m_lightIndex);
		m_quadraticID = glGetUniformLocation(i_programID, _buff);

	}

	void cPointLight::SetLightInitialLocation(glm::vec3 i_position)
	{
		m_transform->Translate(i_position);
	}

}