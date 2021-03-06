#include "PointLight.h"
#include <stdio.h>

#include "Graphics/Graphics.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
namespace Graphics {
	cPointLight::cPointLight(Color i_color, const glm::vec3 & i_position, GLfloat i_range) :
		Range(i_range), cGenLight(i_color)
	{
		Transform.SetTransform(i_position, glm::quat(1, 0, 0, 0), glm::vec3(i_range));
	}
	void cPointLight::Illuminate()
	{
		auto& gLighting = Graphics::GetGlobalLightingData();
		gLighting.pointLights[m_lightIndex].base.uniqueID = UniqueID;
		gLighting.pointLights[m_lightIndex].base.color = LightColor * Intensity;
		gLighting.pointLights[m_lightIndex].base.enableShadow = m_enableShadow;
		gLighting.pointLights[m_lightIndex].position = Transform.Position();
		gLighting.pointLights[m_lightIndex].radius = Range;
		gLighting.pointLights[m_lightIndex].ShadowMapIdx = m_shadowMapIdx;
		gLighting.pointLights[m_lightIndex].ResolutionIdx = m_resolutionIdx;
	}

	void cPointLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

		// Shadow map pass
		m_lightTransformID = glGetUniformLocation(i_programID, "lightMatrices");
		m_farPlaneID = glGetUniformLocation(i_programID, "farPlane");

		assert(GL_NO_ERROR == glGetError());
	}

	void cPointLight::CreateShadowMap(GLuint i_width, GLuint i_height)
	{
		m_lightPrjectionMatrix = glm::perspective(glm::radians(90.f), 1.f, 1.f, Range);
	}

	void cPointLight::SetLightUniformTransform()
	{
		m_lightPrjectionMatrix = glm::perspective(glm::radians(90.f), 1.f, 1.f, Range);

		// order: px, nx, py, ny, pz, nz
		glm::mat4 lightMatrices[6] =
		{
			m_lightPrjectionMatrix * glm::lookAt(Transform.Position(), Transform.Position() + Transform.Right(), -cTransform::WorldUp),
			m_lightPrjectionMatrix * glm::lookAt(Transform.Position(), Transform.Position() - Transform.Right(), -cTransform::WorldUp),
			m_lightPrjectionMatrix * glm::lookAt(Transform.Position(), Transform.Position() + Transform.Up(), cTransform::WorldForward),
			m_lightPrjectionMatrix * glm::lookAt(Transform.Position(), Transform.Position() - Transform.Up(), -cTransform::WorldForward),
			m_lightPrjectionMatrix * glm::lookAt(Transform.Position(), Transform.Position() + Transform.Forward(), -cTransform::WorldUp),
			m_lightPrjectionMatrix * glm::lookAt(Transform.Position(), Transform.Position() - Transform.Forward(), -cTransform::WorldUp),
		};

		glUniformMatrix4fv(m_lightTransformID, 6, GL_FALSE, glm::value_ptr(lightMatrices[0]));
		glUniform1f(m_farPlaneID, Range);
		assert(GL_NO_ERROR == glGetError());
	}

	void cPointLight::UseShadowMap(GLuint i_textureUnit)
	{
		glUniform1i(m_lightShadowMapID, i_textureUnit);
	}


	void cPointLight::CalculateDistToEye(const glm::vec3& i_eyePos)
	{
		m_distToEye = glm::distance(Transform.Position(), i_eyePos);
	}

	glm::mat4 cPointLight::GetViewMatrix() const
	{
		return glm::lookAt(Transform.Position(), Transform.Position() + Transform.Forward(), -cTransform::WorldUp);
	}

}