#include "cFrameBuffer.h"
#include "stdio.h"
#include <string>
namespace Graphics {

	bool cFrameBuffer::Initialize(GLuint i_width, GLuint i_height, ETextureType i_textureType)
	{
		auto result = true;
		m_width = i_height; m_height = i_height;
		GLint _prevBuffer;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_prevBuffer);

		const GLuint mipMapLevel = 0;

		// Generate another frame buffer
		glGenFramebuffers(1, &m_fbo);
		std::string key = "FB_" + std::to_string(m_fbo) + "_ETT_" + std::to_string(i_textureType);

		// Generate render_to_texture texture
		cTexture::s_manager.Load(key, m_renderToTexture, i_textureType, m_width, m_height);
		cTexture* _texture = cTexture::s_manager.Get(m_renderToTexture);
		if (_texture) {

			// bind the frame buffer, it can be read / draw. GL_DRAW_FRAMEBUFFER / GL_READ_FRAMEBUFFER
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

			switch (i_textureType)
			{
			case Graphics::ETT_FRAMEBUFFER_SHADOWMAP:
				// Ref: https://open.gl/framebuffers
				// bind the frame buffer to a texture
				// The second parameter implies that you can have multiple color attachments. 
				//A fragment shader can output different data to any of these by linking out variables to attachments with the glBindFragDataLocation function
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _texture->GetTextureID(), mipMapLevel);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
				break;
			case Graphics::ETT_FRAMEBUFFER_COLOR:

				glDrawBuffer(GL_COLOR_ATTACHMENT0);

				// We need depth too!
				// Use create depth&stencil render buffer in the frame buffer such that we can have a depth and color at the same time
				{
					GLuint rboDepthStencil;
					glGenRenderbuffers(1, &rboDepthStencil);
					glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
					glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
					glFramebufferRenderbuffer(
						GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil
					);
				}
				break;
			default:
				result = false;
				printf("Invalid texture type in creating frame buffer.\n");
				return result;
				break;
			}

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			if (!(result = (status == GL_FRAMEBUFFER_COMPLETE)))
			{
				printf("Frame buffer error in initializing: %i.\n", status);
				return result;
			}

			// cleanup frame buffer, go to previous buffer
			glBindFramebuffer(GL_FRAMEBUFFER, _prevBuffer);
		}
		else {
			printf("Initialize frame buffer error, can not create frame buffer without a texture id\n");
			result = false;
		}
		
		return result;
	}

	void cFrameBuffer::Write()
	{
		// right now, it will write current buffer to this frame buffer 
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	}

	void cFrameBuffer::UnWrite()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void cFrameBuffer::Read(GLenum i_textureID)
	{
		cTexture* _texture = cTexture::s_manager.Get(m_renderToTexture);
		if (_texture) {
			_texture->UseTexture(i_textureID);
		}

	}

	cFrameBuffer::~cFrameBuffer()
	{
		if (m_fbo) {
			glDeleteFramebuffers(1, &m_fbo);
			m_fbo = 0;
		}

		cTexture::s_manager.Release(m_renderToTexture);

	}

	bool cFrameBuffer::IsValid() const
	{
		return (m_fbo != 0);
	}

}
