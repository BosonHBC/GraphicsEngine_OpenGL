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
			: m_diffuseIntensity(Color::White()), m_roughnessIntensity(1.0f), m_ior(1.0f), m_metallicIntensity(1.0f), cMaterial(eMaterialType::MT_PBRMR)
		{}
		static cUniformBuffer& GetUniformBuffer() { return s_PBRMRUniformBlock; }
		virtual ~cMatPBRMR() { CleanUp(); };

		bool Initialize(const std::string& i_path) override;
		bool UpdateUniformVariables(GLuint i_programID) override;
		void UseMaterial() override;
		void CleanUpMaterialBind() override;
		void CleanUp() override;

		Color GetAlbedoIntensity() const { return m_diffuseIntensity; }
		void UpdateAlbedoIntensity(const Color& i_albedo) { m_diffuseIntensity = i_albedo; }
		float GetMetalnessIntensity() const { return m_metallicIntensity; }
		void UpdateMetalnessIntensity(const float& i_metal) { m_metallicIntensity = i_metal; }
		float GetRoughnessIntensity() const { return m_roughnessIntensity; }
		void UpdateRoughnessIntensity(const float& i_rough) { m_roughnessIntensity = i_rough; }
		const Assets::cHandle<cTexture>& GetNormalMapHandle() const { return m_normalMapHandle; }
	private:
		Assets::cHandle<cTexture> m_albedoMapHandle;
		Assets::cHandle<cTexture> m_metallicMapHandle;
		Assets::cHandle<cTexture> m_roughnessMapHandle;
		Assets::cHandle<cTexture> m_normalMapHandle;
		Assets::cHandle<cTexture> m_aoMapHandle;

		Color m_diffuseIntensity = Color(1,1,1);
		float m_metallicIntensity = 1.0f, m_roughnessIntensity = 1.0f;
		glm::vec3 m_ior = glm::vec3(1.0f, 1.0f, 1.0f);

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
