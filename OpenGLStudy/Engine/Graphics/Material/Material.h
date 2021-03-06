/*
	Material describe the light-surface interaction.
	Right now it only support Blinn-Phong Shading.

*/

#pragma once
#include "GL/glew.h"
#include "Graphics/Color/Color.h"
#include "Engine/Graphics/Texture/Texture.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Handle.h"
struct aiMaterial;
namespace Graphics {
	//--------------------------
	// Material enum definition, this enum may be moved to other places
	// When it starts to parse the material file, the subclass of the material is decided by this material.
	enum eMaterialType : uint8_t
	{
		MT_INVALID,
		MT_BLINN_PHONG,
		MT_CUBEMAP,
		MT_UNLIT,
		MT_PBRMR,
		//... will support more in the future
	};
	//--------------------------
	class cMaterial
	{
	public:
		//--------------------------
		// Asset management
		using HANDLE = Assets::cHandle<cMaterial>;
		static Assets::cAssetManager < cMaterial > s_manager;
		static bool Load(const std::string& i_path, cMaterial*& o_material);
		static bool Duplicate(cMaterial* i_src, cMaterial* & o_dest);

		cMaterial(const cMaterial& i_other) : m_matType(i_other.m_matType) {}
		cMaterial& operator = (const cMaterial& i_rhs) { m_matType = i_rhs.m_matType; return *this; }
		virtual ~cMaterial() { m_matType = MT_INVALID; };

		// Actual Initialize function, ready for children class
		virtual bool Initialize(const std::string& i_path) { return false; }
		virtual bool UpdateUniformVariables(GLuint i_programID) { return false; }
		virtual void UseMaterial() {};
		virtual void CleanUpMaterialBind(){}
		virtual void CleanUp() {};


	protected:
		/** private constructor*/
		cMaterial() : m_matType(MT_INVALID) {}
		cMaterial(const eMaterialType& i_matType) : m_matType(i_matType) {}

		// Type of the material
		eMaterialType m_matType;
	
	private:
		static bool LoadMaterialTypeInLUA(const std::string& i_path, eMaterialType& o_matType);
	};

}
