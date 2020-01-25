#define STB_IMAGE_IMPLEMENTATION
#include "assert.h"
#include "stdio.h"
#include "Texture.h"
#include "Tool/stb_image.h"

namespace Graphics {

	cTexture::~cTexture()
	{
		CleanUp();
	}

	bool cTexture::LoadTexture()
	{
		unsigned char* _data = stbi_load(m_filePath, &m_width, &m_height, &m_bitDepth, 0);
		if (!_data) {
			assert(_data, "Fail to load texture data");
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
		return true;
	}

	bool cTexture::LoadTextureA()
	{
		unsigned char* _data = stbi_load(m_filePath, &m_width, &m_height, &m_bitDepth, 0);
		if (!_data) {
			assert(_data, "Fail to load texture data");
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
	}

	void cTexture::UseTexture(int i_textuerLocation)
	{
		// activate the texture unit 0
		glActiveTexture(i_textuerLocation);
		// bind texture to texture unit 0
		// this allow multiples texture to be bound to one texture unit
		// so in one object, it can be multiple texture
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		
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
