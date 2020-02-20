#pragma once
#include "Graphics/Material/Material.h"

namespace Graphics {
	class cMatCubemap : public cMaterial
	{
	public:

		~cMatCubemap() {};

		bool Initialize(const std::string& i_path) override;
		bool UpdateUniformVariables(GLuint i_programID) override;
		void UseMaterial() override;
		void CleanUpMaterialBind() override;
		void CleanUp() override;


	private:
		cMatCubemap(): cMaterial(eMaterialType::MT_CUBEMAP) 
		{}

		GLuint m_cubemapTexID;
	};
}