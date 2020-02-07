#pragma once
#include "Material/Material.h"
namespace Graphics {
	class cMatBlinn : public cMaterial
	{
	public:

		~cMatBlinn() { CleanUp(); };

		bool Initialize(const std::string& i_path) override;
		void UseMaterial() override;
		void CleanUp() override;

		/** Setters */
		void SetDiffuse(const std::string& i_diffusePath);
		void SetSpecular(const std::string& i_specularPath);
		void SetShininess(GLuint i_programID, GLfloat i_shine);
		void SetDiffuseIntensity(GLuint i_programID, Color i_diffuseIntensity);
		void SetSpecularIntensity(GLuint i_programID, Color i_diffuseIntensity);

	private:
		cMatBlinn()
			: m_shininess(0), m_diffuseIntensity(Color::White()), m_specularIntensity(Color::White()), cMaterial(eMaterialType::MT_BLINN_PHONG)
		{}
		cMatBlinn(Color i_diffuseIntensity, Color i_specularIntensity)
			: m_diffuseIntensity(i_diffuseIntensity), m_specularIntensity(i_diffuseIntensity), cMaterial(eMaterialType::MT_BLINN_PHONG)
		{}

		// diffuse texture handle
		Assets::cHandle<cTexture> m_diffuseTextureHandle;
		// specular texture handle
		Assets::cHandle<cTexture> m_specularTextureHandle;

		Color m_diffuseIntensity, m_specularIntensity;
		GLfloat m_shininess;

		GLuint m_shininessID, m_diffuseIntensityID, m_specularIntensityID;

		friend class cMaterial;
	};




}
