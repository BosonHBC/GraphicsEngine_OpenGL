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
		MT_CUBEMAP
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
		//--------------------------

		virtual ~cMaterial() { CleanUp(); };

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
	};

}
