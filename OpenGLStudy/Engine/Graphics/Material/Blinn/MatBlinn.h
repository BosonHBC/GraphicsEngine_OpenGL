#pragma once
#include "Engine/Graphics/Material/Material.h"
#include "Graphics/UniformBuffer/UniformBuffer.h"
namespace Graphics {
	class cMatBlinn : public cMaterial
	{
	public:
		static cUniformBuffer& GetUniformBuffer() { return s_BlinnPhongUniformBlock; }
		~cMatBlinn() { CleanUp(); };

		bool Initialize(const std::string& i_path) override;
		bool UpdateUniformVariables(GLuint i_programID) override;
		void UseMaterial() override;
		void CleanUpMaterialBind() override;
		void CleanUp() override;

		void UpdateDiffuseTexture(const Assets::cHandle<cTexture>& i_other);
		void UpdateSpecularTexture(const Assets::cHandle<cTexture>& i_other);
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
		bool LoadFileFromLua(const std::string& i_path, eMaterialType& o_matType,std::string& o_diffusePath, std::string& o_specularPath, Color& o_diffuseColor, Color& o_specularColor, float& o_shineness);

		// diffuse texture handle
		Assets::cHandle<cTexture> m_diffuseTextureHandle;
		// specular texture handle
		Assets::cHandle<cTexture> m_specularTextureHandle;

		Color m_diffuseIntensity, m_specularIntensity;
		GLfloat m_shininess;

		GLuint m_diffuseTexID, m_specularTexID;
		GLuint m_shininessID, m_diffuseIntensityID, m_specularIntensityID;

		static cUniformBuffer s_BlinnPhongUniformBlock;
		friend class cMaterial;
	};




}
