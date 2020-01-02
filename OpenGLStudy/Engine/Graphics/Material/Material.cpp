#include "Material.h"
#include "Engine/Graphics/Texture/Texture.h"
namespace Graphics {

	void cMaterial::SetDiffuse(const char* i_diffusePath)
	{
		m_diffuse = new cTexture(i_diffusePath);
		m_diffuse->LoadTexture();
	}

	void cMaterial::SetShininess(GLfloat i_shine, GLuint i_programID)
	{
		 m_shininess = i_shine;
		 glGetUniformLocation(i_programID, "shininess");
	}

	void cMaterial::UseMaterial()
	{
		m_diffuse->UseTexture(GL_TEXTURE0);
		glUniform1f(m_shininessLocation, m_shininess);
	}

	void cMaterial::CleanUp()
	{
		if (m_diffuse) {
			delete m_diffuse;
			m_diffuse = nullptr;
		}
	}

}
