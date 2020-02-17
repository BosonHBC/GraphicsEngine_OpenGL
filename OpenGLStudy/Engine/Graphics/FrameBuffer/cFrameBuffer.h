#pragma once

#include "GL/glew.h"

namespace Graphics {
	class cFrameBuffer
	{
	public:
		cFrameBuffer();
		
		virtual bool Initialize(GLuint i_width, GLuint i_height);
		
		// bind fbo and write the texture
		virtual void Write();

		virtual void Read(GLenum i_textureID);

		virtual ~cFrameBuffer();

		/** Getters */
		GLuint GetWidth() const { return m_width; }
		GLuint GetHeight() const { return m_height; }

	protected:
		// fbo: frame buffer object
		// map: render to texture ID
		GLuint m_fbo, m_textureMapID;
		
		// generated map should has same size as the window
		GLuint m_width, m_height;
	};
}
