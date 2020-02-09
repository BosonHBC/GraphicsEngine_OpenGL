#pragma once
#include "Material/Material.h"
namespace Graphics {
	class cMatBlinn : public cMaterial
	{
	public:

		~cMatBlinn() { CleanUp(); };

		bool Initialize(const std::string& i_path, aiMaterial* const i_aiMat) override;
		bool UpdateUniformVariables(GLuint i_programID) override;
		void UseMaterial() override;
		void CleanUpMaterialBind() override;
		void CleanUp() override;

	private:
		cMatBlinn()
			: m_shininess(0), m_diffuseIntensity(Color::White()), m_specularIntensity(Color::White()), cMaterial(eMaterialType::MT_BLINN_PHONG)
		{}
		cMatBlinn(Color i_diffuseIntensity, Color i_specularIntensity)
			: m_diffuseIntensity(i_diffuseIntensity), m_specularIntensity(i_diffuseIntensity), cMaterial(eMaterialType::MT_BLINN_PHONG)
		{}
		/** Setters */
		void SetDiffuse(const std::string& i_diffusePath);
		void SetSpecular(const std::string& i_specularPath);
		void SetShininess(GLfloat i_shine);
		void SetDiffuseIntensity(Color i_diffuseIntensity);
		void SetSpecularIntensity(Color i_diffuseIntensity);

		// LUA Load function
		bool LoadFileFromLua(const std::string& i_path, eMaterialType& o_matType,std::string& o_diffusePath, std::string& o_specularPath, Color& o_IntensityColor, Color& o_SpecularColor, float& o_Shineness);


		// diffuse texture handle
		Assets::cHandle<cTexture> m_diffuseTextureHandle;
		// specular texture handle
		Assets::cHandle<cTexture> m_specularTextureHandle;

		Color m_diffuseIntensity, m_specularIntensity;
		GLfloat m_shininess;

		GLuint m_diffuseTexID, m_specularTexID;
		GLuint m_shininessID, m_diffuseIntensityID, m_specularIntensityID;

		friend class cMaterial;
	};




}
