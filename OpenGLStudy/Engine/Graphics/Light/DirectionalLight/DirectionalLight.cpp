#include "DirectionalLight.h"
#include "glm/gtc/type_ptr.hpp"
#include "Graphics/Graphics.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
namespace Graphics {

	cDirectionalLight::cDirectionalLight(Color i_color, glm::vec3 i_direction)
		:cGenLight(i_color)
	{
		glm::vec3 _right = glm::normalize(glm::cross(i_direction, cTransform::WorldUp));
		glm::vec3 _up = glm::normalize(glm::cross(_right, i_direction));
		glm::quat rot(glm::quatLookAt(i_direction, _up));
		Transform.SetTransform(glm::vec3(0, 0, 0), rot, glm::vec3(1, 1, 1));
	}

	void cDirectionalLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

		m_directionalLightTransformID = glGetUniformLocation(i_programID, "directionalLightTransform");
		m_directionalShadowMapID = glGetUniformLocation(i_programID, "directionalShadowMap");
	}

	void cDirectionalLight::Illuminate()
	{
		auto& gLighting = Graphics::GetGlobalLightingData();
		gLighting.directionalLight.base.color = LightColor * Intensity;
		gLighting.directionalLight.base.enableShadow = m_enableShadow;
		gLighting.directionalLight.direction = Transform.Forward();
	}


	void cDirectionalLight::CreateShadowMap(GLuint i_width, GLuint i_height)
	{
		cGenLight::CreateShadowMap(i_width, i_height);

		// create light projection matrix for directional light
		m_lightPrjectionMatrix = glm::ortho(-512.0f, 512.0f, -512.0f, 512.0f, 1.f, 2000.f);
	
	}

	glm::mat4 cDirectionalLight::GetViewMatrix() const
	{
		return glm::lookAt(Transform.Forward() * 500.f, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	}

	void cDirectionalLight::SetLightUniformTransform()
	{
		glm::mat4 lightTransform = GetProjectionmatrix() * GetViewMatrix();
		// light transform
		glUniformMatrix4fv(m_directionalLightTransformID, 1, GL_FALSE, glm::value_ptr(lightTransform));
		assert(GL_NO_ERROR == glGetError());
	}

	void cDirectionalLight::UseShadowMap(GLuint i_textureUnit)
	{
		// set texture index for the shadow map
		glUniform1i(m_directionalShadowMapID, i_textureUnit);
		assert(glGetError() == GL_NO_ERROR);
	}

}