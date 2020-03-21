#pragma once

#include "GL/glew.h"
#include "Graphics/Texture/Texture.h"

namespace Graphics {
	class cFrameBuffer
	{
	public:
		cFrameBuffer()
			: m_fbo(0), m_rbo(0), m_prevFbo(0), m_width(0), m_height(0) {}

		bool Initialize(GLuint i_width, GLuint i_height, ETextureType i_textureType);
		cFrameBuffer(const cFrameBuffer& i_other):
			m_fbo(i_other.m_fbo), m_rbo(i_other.m_rbo), m_prevFbo(i_other.m_prevFbo),
			m_renderToTexture(i_other.m_renderToTexture), m_width(i_other.m_width), m_height(i_other.m_height)
		{}
		cFrameBuffer& operator = (const cFrameBuffer& i_other);
		// Write current buffer data to this frame buffer
		void Write();
		// Switch back to original frame buffer
		void UnWrite();
		// Use the texture loaded from the frame buffer
		void Read(GLenum i_textureID);

		~cFrameBuffer();

		// Frame buffer must to clean up by the user
		void CleanUp();

		/** Getters */
		bool IsValid() const;
		GLuint GetWidth() const { return m_width; }
		GLuint GetHeight() const { return m_height; }
		cTexture::HANDLE GetTextureHandle() const { return m_renderToTexture; }
		GLuint fbo() const { return m_fbo; }
		GLuint rbo() const { return m_rbo; }
	protected:
		// fbo: frame buffer object
		GLuint m_fbo; // this is necessary
		GLuint m_rbo; // alternative render buffer object if needed.
		GLint m_prevFbo; // stores the previous frame buffer object.
		cTexture::HANDLE m_renderToTexture;

		// generated map should has same size as the window
		GLuint m_width, m_height;
	};
}
