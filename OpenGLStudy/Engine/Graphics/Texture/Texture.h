#pragma once
#include "GL/glew.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Handle.h"

// Download texture in texturer.com
namespace Graphics {
	class cTexture
	{
	public:
		//--------------------------
		// Asset management
		using HANDLE = Assets::cHandle<cTexture>;
		static Assets::cAssetManager < cTexture > s_manager;
		static bool Load(const std::string& i_path, cTexture*& o_texture, bool i_isAlpha);
		//--------------------------

		/** Constructors and destructor */
		~cTexture() { CleanUp(); }

		void UseTexture(int i_textureLocation);
		void CleanUp();

	private:
		/** private constructors */
		cTexture() : m_textureID(0), m_width(0), m_height(0), m_bitDepth(0){}
		cTexture(const char* i_filePath) : m_textureID(0), m_width(0), m_height(0), m_bitDepth(0){}
		
		/** Usage function*/
		bool LoadTexture(const std::string& i_path);
		bool LoadTextureA(const std::string& i_path);

		/** private variables*/
		GLuint m_textureID;
		int m_width, m_height, m_bitDepth;
	};


}
