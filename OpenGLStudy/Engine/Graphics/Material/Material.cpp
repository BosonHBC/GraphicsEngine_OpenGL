#include "Material.h"
#include "Engine/Graphics/Texture/Texture.h"
#include "Engine/Constants/Constants.h"
namespace Graphics {

	void cMaterial::SetDiffuse(const char* i_diffusePath)
	{
		m_diffuse = new cTexture(i_diffusePath);
		if (!m_diffuse->LoadTexture()) {
			delete m_diffuse;
			// Use default texture, which is the white board
			m_diffuse = new cTexture(Constants::CONST_DEFAULT_TEXTURE_PATH);
		}
	}

	void cMaterial::SetShininess(GLfloat i_shine)
	{
		 m_shininess = i_shine;
	}

	void cMaterial::UseMaterial(GLuint i_programID)
	{

		m_diffuse->UseTexture(GL_TEXTURE0);
		glUniform1f(glGetUniformLocation(i_programID, "material.shininess"), m_shininess);
	}

	void cMaterial::CleanUp()
	{
		if (m_diffuse) {
			delete m_diffuse;
			m_diffuse = nullptr;
		}
	}

}
