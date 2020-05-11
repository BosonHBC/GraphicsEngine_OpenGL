#pragma once
#include "Graphics/Material/Material.h"

namespace Graphics {
	class cMatCubemap : public cMaterial
	{
	public:
		virtual ~cMatCubemap() { CleanUp(); };

		cMatCubemap(const cMatCubemap& i_other) : cMaterial(i_other), m_cubemapTexID(m_cubemapTexID)
		{
			cTexture::s_manager.Copy(i_other.m_cubeMapHandle, m_cubeMapHandle);
		}
		cMatCubemap& operator = (const cMatCubemap& i_rhs)
		{
			cMaterial::operator=(i_rhs);
			cTexture::s_manager.Copy(i_rhs.m_cubeMapHandle, m_cubeMapHandle);
			return *this;
		}

		bool Initialize(const std::string& i_path) override;
		bool UpdateUniformVariables(GLuint i_programID) override;
		void UseMaterial() override;
		void CleanUpMaterialBind() override;
		void CleanUp() override;

		/** Getters */
		Assets::cHandle<cTexture> GetCubemapHandle() const { return m_cubeMapHandle; }

		void UpdateCubemap(const Assets::cHandle<cTexture>& i_newTexture);

	private:
		cMatCubemap(): cMaterial(eMaterialType::MT_CUBEMAP) {}
		// cube map texture handle
		Assets::cHandle<cTexture> m_cubeMapHandle;
		bool LoadFileFromLua(const std::string& i_path, std::vector<std::string>& o_textures);

		GLuint m_cubemapTexID = static_cast<GLuint>(-1);

		friend class cMaterial;
	};
}