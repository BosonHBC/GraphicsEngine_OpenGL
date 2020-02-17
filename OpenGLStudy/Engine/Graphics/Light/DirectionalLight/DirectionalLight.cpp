#include "DirectionalLight.h"
#include "glm/gtc/type_ptr.hpp"
namespace Graphics {

	cDirectionalLight::~cDirectionalLight()
	{
	}


	void cDirectionalLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

		m_colorID = glGetUniformLocation(i_programID, "directionalLight.base.color");
		m_enableShadowID = glGetUniformLocation(i_programID, "directionalLight.base.enableShadow");
		m_directionID = glGetUniformLocation(i_programID, "directionalLight.direction");
		m_directionalLightTransformID = glGetUniformLocation(i_programID, "directionalLightTransform");
		m_directionalShadowMapID = glGetUniformLocation(i_programID, "directionalShadowMap");
	}

	void cDirectionalLight::Illuminate()
	{
		glUniform3f(m_colorID, m_color.r, m_color.g, m_color.b);
		glUniform1i(m_enableShadowID, m_enableShadow);
		glUniform3f(m_directionID, m_direction.x, m_direction.y, m_direction.z);

	}

	glm::vec3 cDirectionalLight::Direction(glm::vec3 i_position)
	{
		return m_direction;
	}

	void cDirectionalLight::CreateShadowMap(GLuint i_width, GLuint i_height)
	{
		cGenLight::CreateShadowMap(i_width, i_height);

		// create light projection matrix for directional light
		m_lightPrjectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.f);
	
	}

	glm::mat4 cDirectionalLight::CalculateLightTransform() const
	{
		return m_lightPrjectionMatrix * glm::lookAt(m_direction , glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	}

	void cDirectionalLight::SetLightUniformTransform()
	{
		glm::mat4 lightTransform = CalculateLightTransform();
		// light transform
		glUniformMatrix4fv(m_directionalLightTransformID, 1, GL_FALSE, glm::value_ptr(lightTransform));


	}

	void cDirectionalLight::UseShadowMap(GLuint i_textureUnit)
	{
		// set texture index for the shadow map
		glUniform1i(m_directionalShadowMapID, i_textureUnit);
	}

}