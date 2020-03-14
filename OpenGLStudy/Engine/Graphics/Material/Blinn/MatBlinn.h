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
		void UpdateCubemapTexture(const Assets::cHandle<cTexture>& i_other);
		void UpdateReflectionTexture(const Assets::cHandle<cTexture>& i_other);

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
		void SetNormal(const std::string& i_normalPath);
		void SetShininess(GLfloat i_shine);
		void SetDiffuseIntensity(Color i_diffuseIntensity);
		void SetSpecularIntensity(Color i_diffuseIntensity);

		// LUA Load function
		bool LoadFileFromLua(const std::string& i_path, eMaterialType& o_matType,std::string& o_diffusePath, std::string& o_specularPath, std::string& o_normalPath, Color& o_diffuseColor, Color& o_specularColor, Color& o_environmentIntensity, float& o_shineness);

		// diffuse texture handle
		Assets::cHandle<cTexture> m_diffuseTextureHandle;
		// specular texture handle
		Assets::cHandle<cTexture> m_specularTextureHandle;
		// cube map texture handle
		Assets::cHandle<cTexture> m_cubemapTextureHandle;
		// reflection texture handle
		Assets::cHandle<cTexture> m_reflectionTextureHandle;
		// normal texture handle
		Assets::cHandle<cTexture> m_normalTextureHandle;

		Color m_diffuseIntensity, m_specularIntensity, m_environmentIntensity;
		GLfloat m_shininess;

		GLuint m_diffuseTexID, m_specularTexID, m_normalTexID, m_cubemapTexID, m_reflectionTexID;

		static cUniformBuffer s_BlinnPhongUniformBlock;
		friend class cMaterial;
	};




}
