#define STB_IMAGE_IMPLEMENTATION

#include "stdio.h"
#include "Texture.h"
#include "Tool/stb_image.h"
#include "Assets/PathProcessor.h"
Assets::cAssetManager < Graphics::cTexture > Graphics::cTexture::s_manager;

namespace Graphics {

	bool cTexture::Load(const std::string& i_path, cTexture*& o_texture, ETextureType i_ett /* = FILE*/, const GLuint& i_override_width/* = 0*/, const GLuint& i_override_height /*= 0*/)
	{
		auto result = true;
		cTexture* _texture = nullptr;
		// make sure there is enough memory to allocate a model
		_texture = new (std::nothrow) cTexture();
		if (!(result = _texture)) {
			// Run out of memory
			// TODO: LogError: Out of memory

			return result;
		}
		else
		{
			switch (i_ett)
			{
			case ETT_FILE:
				result = _texture->LoadTextureFromFile(i_path);
				break;
			case ETT_FILE_ALPHA:
				result = _texture->LoadTextureAFromFile(i_path);
				break;
			case ETT_FILE_GRAY:
				result = _texture->LoadTextureGreyFromFile(i_path);
				break;
			case ETT_FRAMEBUFFER_SHADOWMAP:
				result = _texture->LoadShadowMapTexture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FRAMEBUFFER_PLANNER_REFLECTION:
				result = _texture->LoadPlannerReflectionTexture(i_path, i_override_width, i_override_height);
				break;
			case  ETT_CUBEMAP:
				// Cube map is loaded in the cube map material, so it will not load here
				break;
			case ETT_FRAMEBUFFER_CUBEMAP:
				result = _texture->LoadOmniShadowMapTexture(i_path, i_override_width, i_override_height);
				break;
			case  ETT_FRAMEBUFFER_HDR_CUBEMAP:
				result = _texture->LoadHDRCubemapTexture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP:
				result = _texture->LoadHDRMipMapCubemapTexture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FRAMEBUFFER_RG16:
				result = _texture->LoadRG16Texture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FRAMEBUFFER_RGBA16:
				result = _texture->LoadRGBA16Texture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FRAMEBUFFER_RGBA8:
				result = _texture->LoadRGBA8Texture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FRAMEBUFFER_RGB16:
				result = _texture->LoadRGBA16Texture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FRAMEBUFFER_DEPTH16:
				result = _texture->LoadDepth16Texture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FRAMEBUFFER_R16:
				result = _texture->LoadR16Texture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FRAMEBUFFER_RGB32:
				result = _texture->LoadRGB32Texture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FILE_HDR_IMAGE:
				result = _texture->LoadHDRImageFromFile(i_path);
				break;
			case Graphics::ETT_INVALID:
				result = false;
				printf("Load texture error: Invalid texture type: %d in\n", i_ett);
				break;
			default:
				break;
			}
		}


		if (!result) {
			// TODO: Print load texture fail
			delete _texture;
			return result;
		}

		_texture->m_textureType = i_ett;
		o_texture = _texture;

		//TODO: Loading information succeed!
		// No need to display right now
		//printf("Succeed! Loading texture: %s. \n", i_path.c_str());
		assert(GL_NO_ERROR == glGetError());
		return result;
	}

	void cTexture::UnBindTexture(int i_textureLocation, const ETextureType& _textureType)
	{
		glActiveTexture(i_textureLocation);
		assert(glGetError() == GL_NO_ERROR);

		GLenum _glTextureType = GL_TEXTURE_2D;
		// if the texture is a cube map, use GL_TEXTURE_CUBE_MAP
		if (_textureType == ETT_CUBEMAP
			|| _textureType == ETT_FRAMEBUFFER_CUBEMAP
			|| _textureType == ETT_FRAMEBUFFER_HDR_CUBEMAP
			|| _textureType == ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP)
			_glTextureType = GL_TEXTURE_CUBE_MAP;
		
		glBindTexture(_glTextureType, 0);
		assert(glGetError() == GL_NO_ERROR);
	}

	bool cTexture::LoadTextureFromFile(const std::string& i_path)
	{
		unsigned char* _data = stbi_load(i_path.c_str(), &m_width, &m_height, &m_bitDepth, STBI_rgb);
		if (!_data) {
			printf("Fail to load texture data!\n");
			return false;
		}

		// generate texture and assign with an id
		glGenTextures(1, &m_textureID);
		// Bind it with a texture 2d
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		// Set up texture wrapping in s,t axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set up texture filtering for looking closer
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		// Set up texture filtering for looking further
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// texture type
		// mipmap level
		// texture format to load
		// width / height
		// legacy for border
		// texture format to become
		// load in data type 
		// data it-self
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, _data);
		// Generate mip map
		glGenerateMipmap(GL_TEXTURE_2D);

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);

		// clear data of stb_image
		stbi_image_free(_data);

		auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			printf("OpenGL failed to load texture %s, with error ID: %d.\n", i_path.c_str(), errorCode);
		}
		return true;
	}

	bool cTexture::LoadTextureAFromFile(const std::string& i_path)
	{
		unsigned char* _data = stbi_load(i_path.c_str(), &m_width, &m_height, &m_bitDepth, STBI_rgb_alpha);
		if (!_data) {
			// TODO: Print load texture fail
			printf("Fail to load texture data!\n");
			return false;
		}

		// generate texture and assign with an id
		glGenTextures(1, &m_textureID);
		// Bind it with a texture 2d
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		// Set up texture wrapping in s,t axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set up texture filtering for looking closer
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// Set up texture filtering for looking further
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// texture type
		// mipmap level
		// texture format to load
		// width / height
		// legacy for border
		// texture format to become
		// load in data type 
		// data it-self
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
		// Generate mip map
		glGenerateMipmap(GL_TEXTURE_2D);

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);

		// clear data of stb_image
		stbi_image_free(_data);
		return true;
	}

	bool cTexture::LoadTextureGreyFromFile(const std::string& i_path)
	{
		unsigned char* _data = stbi_load(i_path.c_str(), &m_width, &m_height, &m_bitDepth, STBI_grey);
		if (!_data) {
			// TODO: Print load texture fail
			printf("Fail to load texture data of type ETT_FILE_GREY!\n");
			return false;
		}

		// generate texture and assign with an id
		glGenTextures(1, &m_textureID);
		// Bind it with a texture 2d
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		// Set up texture wrapping in s,t axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set up texture filtering for looking closer
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// Set up texture filtering for looking further
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// texture type
		// mipmap level
		// texture format to load
		// width / height
		// legacy for border
		// texture format to become
		// load in data type 
		// data it-self
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, _data);
		// Generate mip map
		glGenerateMipmap(GL_TEXTURE_2D);

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);

		// clear data of stb_image
		stbi_image_free(_data);

		auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			printf("OpenGL failed to load texture %s, with error ID: %d.\n", i_path.c_str(), errorCode);
			assert(false);
		}
		return true;

	}

	bool cTexture::LoadHDRImageFromFile(const std::string& i_path)
	{
		float* _data = stbi_loadf(i_path.c_str(), &m_width, &m_height, &m_bitDepth, 0);
		if (!_data) {
			printf("Fail to load texture data!\n");
			return false;
		}

		// generate texture and assign with an id
		glGenTextures(1, &m_textureID);
		// Bind it with a texture 2d
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		// Set up texture wrapping in s,t axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Set up texture filtering for looking closer
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// Set up texture filtering for looking further
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// texture type
		// mipmap level
		// texture format to load
		// width / height
		// legacy for border
		// texture format to become
		// load in data type 
		// data it-self
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, _data);

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);

		// clear data of stb_image
		stbi_image_free(_data);

		auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			printf("OpenGL failed to load texture %s, with error ID: %d.\n", i_path.c_str(), errorCode);
		}
		return true;
	}

	bool cTexture::LoadShadowMapTexture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		const GLuint mipMapLevel = 0;
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		// allocate space for the texture with null data fill in
		glTexImage2D(GL_TEXTURE_2D, mipMapLevel, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

		// Set up texture wrapping in s,t axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		const float borderColor[4] = { 1.f,1.f,1.f,1.f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Set up texture filtering for looking closer
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		// Set up texture filtering for looking further
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
		return result;
	}

	bool cTexture::LoadOmniShadowMapTexture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		// This is for omniShadowMap, Generate cube map texture here

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

		for (size_t i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
				m_width, m_height, 0,
				GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}
		// Set up texture wrapping in s,t,r axis
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// Set up texture filtering for looking closer
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// Set up texture filtering for looking further
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// unbind texture
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		assert(GL_NO_ERROR == glGetError());
		return result;
	}

	bool cTexture::LoadHDRCubemapTexture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		// This is for omniShadowMap, Generate cube map texture here

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

		for (size_t i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
				m_width, m_height, 0,
				GL_RGB, GL_FLOAT, nullptr);
		}
		// Set up texture wrapping in s,t,r axis
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// Set up texture filtering for looking closer
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		// Set up texture filtering for looking further
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// unbind texture
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		assert(GL_NO_ERROR == glGetError());
		return result;
	}

	bool cTexture::LoadHDRMipMapCubemapTexture(const std::string& i_type_id, const GLuint& i_baseWidth, const GLuint& i_baseHeight)
	{
		auto result = true;
		m_width = i_baseWidth;
		m_height = i_baseHeight;

		// This is for omniShadowMap, Generate cube map texture here

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

		for (size_t i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
				m_width, m_height, 0,
				GL_RGB, GL_FLOAT, nullptr);
		}
		// Set up texture wrapping in s,t,r axis
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// Set up texture filtering for looking closer
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  // enable pre-filter mipmap sampling (combating visible dots artifact)
		// Set up texture filtering for looking further
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// generate mip-maps for the cube map so OpenGL automatically allocates the required memory.
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// unbind texture
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		assert(GL_NO_ERROR == glGetError());
		return result;
	}

	bool cTexture::LoadRG16Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		glGenTextures(1, &m_textureID);

		// pre-allocate enough memory for the LUT texture.
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, m_width, m_width, 0, GL_RG, GL_FLOAT, nullptr);

		// use GL_CLAMP_TO_EDGE to prevent from edge sampling artifacts
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		assert(GL_NO_ERROR == glGetError());
		
		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
		return result;
	}

	bool cTexture::LoadRGBA16Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, i_width, i_height, 0, GL_RGBA, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		assert(GL_NO_ERROR == glGetError());

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
		return result;
	}

	bool cTexture::LoadRGBA8Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, i_width, i_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		assert(GL_NO_ERROR == glGetError());

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
		return result;
	}

	bool cTexture::LoadRGB16Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, i_width, i_height, 0, GL_RGB, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		assert(GL_NO_ERROR == glGetError());

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
		return result;
	}

	bool cTexture::LoadDepth16Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		// allocate space for the texture with null data fill in
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

		// Set up texture wrapping in s,t axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		assert(GL_NO_ERROR == glGetError());

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
		return result;
	}

	bool cTexture::LoadR16Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RGB, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		assert(GL_NO_ERROR == glGetError());

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
		return result;
	}

	bool cTexture::LoadRGB32Texture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_width, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		assert(GL_NO_ERROR == glGetError());

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
		return result;
	}

	bool cTexture::LoadPlannerReflectionTexture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
	{
		auto result = true;
		m_width = i_width;
		m_height = i_height;

		const GLuint mipMapLevel = 0;
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		// allocate space for the texture with null data fill in
		glTexImage2D(GL_TEXTURE_2D, mipMapLevel, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, nullptr);

		// Set up texture wrapping in s,t axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		const float borderColor[4] = { 1.f,1.f,1.f,1.f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Set up texture filtering for looking closer
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		// Set up texture filtering for looking further
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		assert(GL_NO_ERROR == glGetError());
		
		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
		return result;
	}

	bool cTexture::LoadCubemap(const std::vector<std::string>& i_paths)
	{
		auto result = true;

		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
		assert(glGetError() == GL_NO_ERROR);
		unsigned char* _data = nullptr;
		if (i_paths.size() < 6) {
			result = false;
			printf("Cube map texture paths count is smaller than 6\n");
			return result;
		}

		for (int i = 0; i < 6; ++i)
		{
			std::string _path = Assets::ProcessPathTex(i_paths[i]);
			_data = stbi_load(_path.c_str(), &m_width, &m_height, &m_bitDepth, 0);
			if (_data) {
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, _data);
				stbi_image_free(_data);
			}
			else {
				result = false;
				printf("Fail to load cube map: %s\n", i_paths[i].c_str());
				stbi_image_free(_data);
				return result;
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		const auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR) {
			printf("Cube map error: fail to load cube map.\n");
		}

		return result;
	}

	void cTexture::UseTexture(int i_textureLocation)
	{
		// activate the texture unit at i_textureLocation
		glActiveTexture(i_textureLocation);
		assert(glGetError() == GL_NO_ERROR);
		GLenum _textureType = GL_TEXTURE_2D;
		// if the texture is a cube map, use GL_TEXTURE_CUBE_MAP
		if (m_textureType == ETT_CUBEMAP
			|| m_textureType == ETT_FRAMEBUFFER_CUBEMAP 
			|| m_textureType == ETT_FRAMEBUFFER_HDR_CUBEMAP
			|| m_textureType == ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP)
			_textureType = GL_TEXTURE_CUBE_MAP;
		// bind texture to texture unit i_textureLocation
		// this allow multiples texture to be bound to one shader
		// so in one object, it can be multiple texture
		glBindTexture(_textureType, m_textureID);
		assert(glGetError() == GL_NO_ERROR);
	}

	void cTexture::CleanUpTextureBind(int i_textureLocation)
	{
		cTexture::UnBindTexture(i_textureLocation, m_textureType);
	}

	void cTexture::CleanUp()
	{
		if (m_textureID)
		{
			glDeleteTextures(1, &m_textureID);
			m_textureID = 0;
			assert(GL_NO_ERROR == glGetError());
		}
		m_width = 0;
		m_height = 0;
		m_bitDepth = 0;
	}

}
