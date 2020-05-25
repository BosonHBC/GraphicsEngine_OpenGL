#pragma once
#include "GL/glew.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Handle.h"

// Download texture in texturer.com
namespace Graphics {

	// This enum determines the type of the texture
	enum ETextureType : uint8_t
	{
		ETT_FILE = 0,
		ETT_FILE_ALPHA = 1,
		ETT_FRAMEBUFFER_SHADOWMAP = 2,
		ETT_FRAMEBUFFER_STENCIL = 3,
		ETT_CUBEMAP = 4,
		ETT_FRAMEBUFFER_OMNI_SHADOWMAP = 5,
		ETT_FILE_GRAY = 6,
		ETT_FRAMEBUFFER_HDR_CUBEMAP = 7,
		ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP = 8,
		ETT_FRAMEBUFFER_RG16 = 9,
		ETT_FILE_HDR_IMAGE = 10,
		ETT_FRAMEBUFFER_RGBA16 = 11,
		ETT_FRAMEBUFFER_RGBA8 = 12,
		ETT_FRAMEBUFFER_RGB16 = 13,
		ETT_FRAMEBUFFER_DEPTH16 = 14,
		ETT_FRAMEBUFFER_R16 = 15,
		ETT_FRAMEBUFFER_RGB32 = 16,
		ETT_FRAMEBUFFER_RGB8 = 17,
		ETT_FRAMEBUFFER_RGBA32 = 18,
		ETT_INVALID = 0xff
	};

	class cTexture
	{
	public:
		//--------------------------
		// Asset management
		using HANDLE = Assets::cHandle<cTexture>;
		static Assets::cAssetManager < cTexture > s_manager;
		static bool Load(const std::string& i_path, cTexture*& o_texture, ETextureType i_ett = ETT_FILE, const GLuint& i_override_width =0, const GLuint& i_override_height = 0);
		//--------------------------

		static void UnBindTexture(int i_textureLocation, const ETextureType& _textureType);

		/** Constructors and destructor */
		~cTexture() { CleanUp(); }

		void UseTexture(int i_textureLocation);
		void CleanUpTextureBind(int i_textureLocation);
		void CleanUp();

		void SetTextureID(GLuint i_textureID) { m_textureID = i_textureID; }
		GLuint GetTextureID() const { return m_textureID; }

		// Load Cube map externally
		bool LoadCubemap(const std::vector<std::string>& i_paths);

	private:
		/** private constructors */
		cTexture() : m_textureID(0), m_width(0), m_height(0), m_bitDepth(0){}
		cTexture(const char* i_filePath) : m_textureID(0), m_width(0), m_height(0), m_bitDepth(0){}
		
		/** Usage function*/
		bool LoadTextureFromFile(const std::string& i_path); // 8 bits channel
		bool LoadTextureAFromFile(const std::string& i_path); // 8 bits channel
		bool LoadTextureGreyFromFile(const std::string& i_path); // 8 bits channel
		bool LoadHDRImageFromFile(const std::string& i_path); // 16 or 32 bits per channel
		// Load render_to_texture from frame buffer
		bool LoadShadowMapTexture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);
		bool LoadOmniShadowMapTexture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);
		bool LoadHDRCubemapTexture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height); // assume the bit width is 16 bits per channel
		bool LoadHDRMipMapCubemapTexture(const std::string& i_type_id, const GLuint& i_baseWidth, const GLuint& i_baseHeight);
		bool LoadRG16Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);			// brdf lookup texture/
		bool LoadRGBA16Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);		// deferred shading buffers
		bool LoadRGBA8Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);		// 8-bit rgba color buffer
		bool LoadRGB16Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);		// hdr buffer
		bool LoadDepth24Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);		// depth buffer
		bool LoadR16Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);				// ssao textures
		bool LoadRGB32Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);		// noise texture
		bool LoadRGB8Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);			// selection buffer
		bool LoadRGBA32Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);		// tiled lighting image
		// Load color format texture from frame buffer
		bool LoadStencilTexture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height);

		/** private variables*/
		GLuint m_textureID;
		ETextureType m_textureType;
		int m_width, m_height, m_bitDepth;
	};


}
