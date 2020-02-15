#include "cFrameBuffer.h"
#include "stdio.h"

namespace Graphics {
	cFrameBuffer::cFrameBuffer()
	{
		m_fbo = m_textureMapID = m_width = m_height = 0;
	}


	bool cFrameBuffer::Initialize(GLuint i_width, GLuint i_height)
	{
		auto result = true;
		m_width = i_height; m_height = i_height;

		// Generate another frame buffer
		glGenFramebuffers(1, &m_fbo);

		// Generate render_to_texture texture buffer
		glGenTextures(1, &m_textureMapID);
		glBindTexture(GL_TEXTURE_2D, m_textureMapID);

		GLuint mipMapLevel = 0;
		// allocate space for the texture with null data fill in
		glTexImage2D(GL_TEXTURE_2D, mipMapLevel, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		
		// Set up texture wrapping in s,t axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set up texture filtering for looking closer
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// Set up texture filtering for looking further
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// bind the frame buffer, it can be read / draw. GL_DRAW_FRAMEBUFFER / GL_READ_FRAMEBUFFER
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		// write depth map
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureMapID, mipMapLevel);
		
		// no need to draw color values
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (!(result = (status == GL_FRAMEBUFFER_COMPLETE)))
		{
			printf("Frame buffer error in initializing: %i.\n", status);
			return result;
		}

		// cleanup frame buffer, go to default frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return result;
	}

	void cFrameBuffer::Write()
	{
		// right now, it will draw to this frame buffer 
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	}

	void cFrameBuffer::Read(GLenum i_textureID)
	{
		glActiveTexture(i_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureMapID);
	}

	cFrameBuffer::~cFrameBuffer()
	{
		if (m_fbo) {
			glDeleteFramebuffers(1, &m_fbo);
			m_fbo = 0;
		}
		if (m_textureMapID) {
			glDeleteTextures(1, &m_textureMapID);
			m_textureMapID = 0;
		}
	}

}
