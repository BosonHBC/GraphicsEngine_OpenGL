#pragma once
#include "Graphics/Material/Material.h"
#include "Graphics/UniformBuffer/UniformBuffer.h"
#include "glm/vec3.hpp"
namespace Graphics 
{
	class cMatPBRMR : public cMaterial
	{
	public:
		cMatPBRMR()
			: DiffuseIntensity(Color::White()), RoughnessIntensity(1.0f), IoR(1.0f), MetallicIntensity(1.0f), cMaterial(eMaterialType::MT_PBRMR)
		{}
		static cUniformBuffer& GetUniformBuffer() { return s_PBRMRUniformBlock; }

		cMatPBRMR(const cMatPBRMR& i_other);
		cMatPBRMR& operator = (const cMatPBRMR& i_rhs);

		virtual ~cMatPBRMR() { CleanUp(); };

		bool Initialize(const std::string& i_path) override;
		bool UpdateUniformVariables(GLuint i_programID) override;
		void UseMaterial() override;
		void CleanUpMaterialBind() override;
		void CleanUp() override;

		void UpdateAlbedoIntensity(const Color& i_albedo) { DiffuseIntensity = i_albedo; }
		void UpdateMetalnessIntensity(const float& i_metal) { MetallicIntensity = i_metal; }
		void UpdateRoughnessIntensity(const float& i_rough) { RoughnessIntensity = i_rough; }
		const Assets::cHandle<cTexture>& GetNormalMapHandle() const { return m_normalMapHandle; }

		Color DiffuseIntensity = Color(1, 1, 1);
		float MetallicIntensity = 1.0f, RoughnessIntensity = 1.0f;
		glm::vec3 IoR = glm::vec3(1.0f, 1.0f, 1.0f);

	private:
		Assets::cHandle<cTexture> m_albedoMapHandle;
		Assets::cHandle<cTexture> m_metallicMapHandle;
		Assets::cHandle<cTexture> m_roughnessMapHandle;
		Assets::cHandle<cTexture> m_normalMapHandle;
		Assets::cHandle<cTexture> m_aoMapHandle;



		GLuint m_albedoID = static_cast<GLuint>(-1), m_metallicID = static_cast<GLuint>(-1), m_roughnessID = static_cast<GLuint>(-1), m_normalID = static_cast<GLuint>(-1), m_aoID = static_cast<GLuint>(-1);

		/* private functions */

		bool LoadFileFromLua(const std::string& i_path, std::string& o_albedoPath, std::string& o_metallicPath, std::string& o_roughnessPath, std::string& o_normalPath, std::string& o_aoPath, Color& o_diffuseIntensity, float& o_metallicIntensity, float& o_roughnessIntensity, glm::vec3& o_ior);
		bool SetAlbedo(const std::string& i_path);
		bool SetMetallic(const std::string& i_path);
		bool SetRoughness(const std::string& i_path);
		bool SetNormal(const std::string& i_path);
		bool SetAO(const std::string& i_path);


		/* private static variable */
		static cUniformBuffer s_PBRMRUniformBlock;
		friend class cMaterial;
	};
}
