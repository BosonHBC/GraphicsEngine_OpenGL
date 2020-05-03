#pragma once
#include "Graphics/Material/Material.h"

namespace Graphics {
	class cMatUnlit : public cMaterial
	{
	public:

		cMatUnlit(const cMatUnlit& i_other) : cMaterial(i_other), m_unlitColor(i_other.m_unlitColor), m_unLitColorID() { }
		cMatUnlit& operator = (const cMatUnlit& i_rhs)
		{
			cMaterial::operator=(i_rhs);
			m_unlitColor = i_rhs.m_unlitColor;
			m_unLitColorID = i_rhs.m_unLitColorID;
			return *this;
		}
		virtual ~cMatUnlit() { CleanUp(); };

		bool Initialize(const std::string& i_path) override;
		bool UpdateUniformVariables(GLuint i_programID) override;
		void UseMaterial() override;
		void CleanUpMaterialBind() override;
		void CleanUp() override;
		void SetUnlitColor(const Color& i_color) { m_unlitColor = i_color; }
	private:
		cMatUnlit() : cMaterial(eMaterialType::MT_UNLIT)
		{}
		bool LoadFileFromLua(const std::string& i_path, Color& o_unlitColor);

		Color m_unlitColor;
		GLuint m_unLitColorID;
		friend class cMaterial;
	};
}