#include "MatBlinn.h"
#include "Engine/Constants/Constants.h"

namespace Graphics {
	void cMatBlinn::SetDiffuse(const std::string& i_diffusePath)
	{
		auto result = true;
		if (!(result = cTexture::s_manager.Load(i_diffusePath, m_diffuseTextureHandle, false))) {
			if (result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_TEXTURE, m_diffuseTextureHandle, false))
			{
				//TODO: Use default texture, which is the white board

			}
			else {
				//TODO: print Fail to load default texture

			}
		}
	}
	void cMatBlinn::UseMaterial(GLuint i_programID)
	{
		glUniform1f(m_shininessID, m_shininess);
		glUniform3f(m_diffuseIntensityID, m_diffuseIntensity.r, m_diffuseIntensity.g, m_diffuseIntensity.b);
		glUniform3f(m_specularIntensityID, m_specularIntensity.r, m_specularIntensity.g, m_specularIntensity.b);

	}

	void cMatBlinn::SetShininess(GLuint i_programID, GLfloat i_shine)
	{
		m_shininess = i_shine;
		m_shininessID = glGetUniformLocation(i_programID, "material.shininess");
	}

	void cMatBlinn::SetDiffuseIntensity(GLuint i_programID, Color i_diffuseIntensity)
	{
		m_diffuseIntensity = i_diffuseIntensity;
		m_diffuseIntensityID = glGetUniformLocation(i_programID, "material.kd");
	}

	void cMatBlinn::SetSpecularIntensity(GLuint i_programID, Color i_specularIntensity)
	{
		m_specularIntensity = i_specularIntensity;
		m_specularIntensityID = glGetUniformLocation(i_programID, "material.ks");
	}
}