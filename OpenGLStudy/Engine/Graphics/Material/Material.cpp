#include "Material.h"
#include "Engine/Graphics/Texture/Texture.h"
#include "Engine/Constants/Constants.h"

namespace Graphics {

	void cMaterial::SetDiffuse(const std::string& i_diffusePath)
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

	void cMaterial::SetShininess(GLuint i_programID, GLfloat i_shine)
	{
		 m_shininess = i_shine;
		 m_shininessID = glGetUniformLocation(i_programID, "material.shininess");
	}

	void cMaterial::SetDiffuseIntensity(GLuint i_programID, Color i_diffuseIntensity)
	{
		m_diffuseIntensity = i_diffuseIntensity;
		m_diffuseIntensityID = glGetUniformLocation(i_programID, "material.kd");
	}

	void cMaterial::SetSpecularIntensity(GLuint i_programID, Color i_specularIntensity)
	{
		m_specularIntensity = i_specularIntensity;
		m_specularIntensityID = glGetUniformLocation(i_programID, "material.ks");
	}

	void cMaterial::UseMaterial(GLuint i_programID)
	{
		//m_diffuseTexture->UseTexture(GL_TEXTURE0);
		glUniform1f(m_shininessID  , m_shininess);
		glUniform3f(m_diffuseIntensityID, m_diffuseIntensity.r, m_diffuseIntensity.g, m_diffuseIntensity.b);
		glUniform3f(m_specularIntensityID, m_specularIntensity.r, m_specularIntensity.g, m_specularIntensity.b);

	}

	void cMaterial::CleanUp()
	{
		// Release diffuse texture
		cTexture::s_manager.Release(m_diffuseTextureHandle);
	}

}
