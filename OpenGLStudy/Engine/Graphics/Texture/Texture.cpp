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
			case ETT_FRAMEBUFFER_SHADOWMAP:
				result = _texture->LoadShadowMapTexture(i_path, i_override_width, i_override_height);
				break;
			case ETT_FRAMEBUFFER_COLOR:
				result = _texture->LoadRGBTexture(i_path, i_override_width, i_override_height);
				break;
			case  ETT_CUBEMAP:
				// Cube map is loaded in the cube map material, so it will not load here
				break;
			case ETT_FRAMEBUFFER_CUBEMAP:
				result = _texture->LoadOmniShadowMapTexture(i_path, i_override_width, i_override_height);
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
		printf("Succeed! Loading texture: %s. \n", i_path.c_str());

		return result;
	}

	bool cTexture::LoadTextureFromFile(const std::string& i_path)
	{
		unsigned char* _data = stbi_load(i_path.c_str(), &m_width, &m_height, &m_bitDepth, 0);
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
		unsigned char* _data = stbi_load(i_path.c_str(), &m_width, &m_height, &m_bitDepth, 0);
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

	bool cTexture::LoadRGBTexture(const std::string& i_type_id, const GLuint& i_width, const GLuint& i_height)
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
			|| m_textureType == ETT_FRAMEBUFFER_CUBEMAP)
			_textureType = GL_TEXTURE_CUBE_MAP;
		// bind texture to texture unit i_textureLocation
		// this allow multiples texture to be bound to one shader
		// so in one object, it can be multiple texture
		glBindTexture(_textureType, m_textureID);
		assert(glGetError() == GL_NO_ERROR);
	}

	void cTexture::CleanUpTextureBind(int i_textureLocation)
	{
		glActiveTexture(i_textureLocation);

		assert(glGetError() == GL_NO_ERROR);
		GLenum _textureType = GL_TEXTURE_2D;
		// if the texture is a cube map, use GL_TEXTURE_CUBE_MAP
		if (m_textureType == ETT_CUBEMAP
			|| m_textureType == ETT_FRAMEBUFFER_CUBEMAP)
			_textureType = GL_TEXTURE_CUBE_MAP;
		glBindTexture(_textureType, 0);
		assert(glGetError() == GL_NO_ERROR);
	}

	void cTexture::CleanUp()
	{
		glDeleteTextures(1, &m_textureID);
		m_textureID = 0;
		m_width = 0;
		m_height = 0;
		m_bitDepth = 0;
	}

}
