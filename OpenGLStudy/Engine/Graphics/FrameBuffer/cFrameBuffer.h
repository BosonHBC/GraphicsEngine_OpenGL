#pragma once

#include "GL/glew.h"
#include "Graphics/Texture/Texture.h"

namespace Graphics {
	class cFrameBuffer
	{
	public:
		cFrameBuffer()
			: m_fbo(0), m_width(0), m_height(0){}
		
		virtual bool Initialize(GLuint i_width, GLuint i_height, ETextureType i_textureType);
		
		// bind fbo and write the texture
		virtual void Write();

		virtual void Read(GLenum i_textureID);

		virtual ~cFrameBuffer();

		/** Getters */
		GLuint GetWidth() const { return m_width; }
		GLuint GetHeight() const { return m_height; }

	protected:
		cFrameBuffer(const cFrameBuffer& i_other) = delete;
		cFrameBuffer& operator = (const cFrameBuffer& i_other) = delete;

		// fbo: frame buffer object
		// map: render to texture ID
		GLuint m_fbo;
		cTexture::HANDLE m_renderToTexture;
		
		// generated map should has same size as the window
		GLuint m_width, m_height;
	};
}
