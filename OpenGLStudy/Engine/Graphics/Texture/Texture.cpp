#define STB_IMAGE_IMPLEMENTATION

#include "stdio.h"
#include "Texture.h"
#include "Tool/stb_image.h"
#include "Constants/Constants.h"
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
			case Graphics::ETT_FILE:
				result = _texture->LoadTextureFromFile(i_path);
				break;
			case Graphics::ETT_FILE_ALPHA:
				result = _texture->LoadTextureAFromFile(i_path);
				break;
			case Graphics::ETT_FRAMEBUFFER_SHADOWMAP:
				result = _texture->LoadShadowMapTexture(i_path, i_override_width, i_override_height);
				break;
			case Graphics::ETT_FRAMEBUFFER_COLOR:
				result = _texture->LoadRGBTexture(i_path, i_override_width, i_override_height);
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
			printf("OpenGL failed to load texture %s, with error ID: %d.\n", i_path, errorCode);
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

	void cTexture::UseTexture(int i_textureLocation)
	{
		// activate the texture unit at i_textureLocation
		glActiveTexture(i_textureLocation);
		// bind texture to texture unit i_textureLocation
		// this allow multiples texture to be bound to one shader
		// so in one object, it can be multiple texture
		glBindTexture(GL_TEXTURE_2D, m_textureID);
	}

	void cTexture::CleanUpTextureBind(int i_textureLocation)
	{
		glActiveTexture(i_textureLocation);
		glBindTexture(GL_TEXTURE_2D, 0);
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
