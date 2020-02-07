#pragma once
#include "Material/Material.h"
namespace Graphics {
	class cMatBlinn : public cMaterial
	{
	public:

		~cMatBlinn() { CleanUp(); };

		void UseMaterial(GLuint i_programID) override;
		void CleanUp() override;

		/** Setters */
		void SetDiffuse(const std::string& i_diffusePath);
		void SetShininess(GLuint i_programID, GLfloat i_shine);
		void SetDiffuseIntensity(GLuint i_programID, Color i_diffuseIntensity);
		void SetSpecularIntensity(GLuint i_programID, Color i_diffuseIntensity);

	private:
		cMatBlinn()
			: m_shininess(0), m_diffuseIntensity(Color::White()), m_specularIntensity(Color::White()), m_matType(eMaterialType::MT_BLINN_PHONG)
		{}
		cMatBlinn(Color i_diffuseIntensity, Color i_specularIntensity)
			: m_diffuseIntensity(i_diffuseIntensity), m_specularIntensity(i_diffuseIntensity)
		{}

		// diffuse texture handle
		Assets::cHandle<cTexture> m_diffuseTextureHandle;
		// specular texture handle
		Assets::cHandle<cTexture> m_specularTextureHandle;

		Color m_diffuseIntensity, m_specularIntensity;
		GLfloat m_shininess;

		GLuint m_shininessID, m_diffuseIntensityID, m_specularIntensityID;
	};

	void cMatBlinn::CleanUp()
	{
		cTexture::s_manager.Release(m_diffuseTextureHandle);
		cTexture::s_manager.Release(m_specularTextureHandle);

	}


}
