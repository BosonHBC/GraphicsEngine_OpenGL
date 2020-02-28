#pragma once
#include "Graphics/Material/Material.h"

namespace Graphics {
	class cMatUnlit : public cMaterial
	{
	public:
		~cMatUnlit() { CleanUp(); };

		bool Initialize(const std::string& i_path) override;
		bool UpdateUniformVariables(GLuint i_programID) override;
		void UseMaterial() override;
		void CleanUpMaterialBind() override;
		void CleanUp() override;

	private:
		cMatUnlit() : cMaterial(eMaterialType::MT_UNLIT)
		{}
		bool LoadFileFromLua(const std::string& i_path, Color& o_unlitColor);

		Color m_unlitColor;
		GLuint m_unLitColorID;
		friend class cMaterial;
	};
}