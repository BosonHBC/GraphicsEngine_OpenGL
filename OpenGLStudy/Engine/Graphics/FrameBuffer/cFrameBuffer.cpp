#include "cFrameBuffer.h"
#include "stdio.h"
#include <string>
#include "assert.h"
#include "Application/Application.h"
#include "Application/Window/Window.h"

namespace Graphics {

	bool cFrameBuffer::Initialize(GLuint i_width, GLuint i_height, ETextureType i_textureType)
	{
		auto result = true;
		m_width = i_width; m_height = i_height;
		// record the previous frame buffer object
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_prevFbo);

		const GLuint mipMapLevel = 0;

		// Generate another frame buffer
		glGenFramebuffers(1, &m_fbo);
		std::string key = "FB_" + std::to_string(m_fbo) + "_ETT_" + std::to_string(i_textureType);
		assert(GL_NO_ERROR == glGetError());

		// Generate render_to_texture texture
		cTexture::s_manager.Load(key, m_renderToTexture, i_textureType, m_width, m_height);
		cTexture* _texture = cTexture::s_manager.Get(m_renderToTexture);
		if (_texture) {
			// bind the frame buffer, it can be read / draw. GL_DRAW_FRAMEBUFFER / GL_READ_FRAMEBUFFER
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

			switch (i_textureType)
			{
			case ETT_FRAMEBUFFER_SHADOWMAP:
				// Ref: https://open.gl/framebuffers
				// bind the frame buffer to a texture
				// The second parameter implies that you can have multiple color attachments. 
				//A fragment shader can output different data to any of these by linking out variables to attachments with the glBindFragDataLocation function
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _texture->GetTextureID(), mipMapLevel);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
				assert(GL_NO_ERROR == glGetError());
				break;
			case ETT_FRAMEBUFFER_PLANNER_REFLECTION:

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture->GetTextureID(), mipMapLevel);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				assert(GL_NO_ERROR == glGetError());
				// We need depth too!
				// Use render buffer with frame buffer such that we can have a depth and color at the same time
				{
					glGenRenderbuffers(1, &m_rbo);
					glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
					glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
					assert(GL_NO_ERROR == glGetError());
				}
				break;
			case ETT_FRAMEBUFFER_CUBEMAP:
				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _texture->GetTextureID(), mipMapLevel);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
				assert(GL_NO_ERROR == glGetError());
				break;

			case ETT_FRAMEBUFFER_HDR_CUBEMAP:
			case ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP:
				// We are not going to bind texture to the frame buffer here because we need to render 6 faces face by face
				// ... glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _texture->GetTextureID(), mipMapLevel);
				// Need depth too here	
				glGenRenderbuffers(1, &m_rbo);
				glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

				assert(GL_NO_ERROR == glGetError());
				break;

			case ETT_FRAMEBUFFER_RG16:
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture->GetTextureID(), 0);
				/*glGenRenderbuffers(1, &m_rbo);
				glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);*/

				assert(GL_NO_ERROR == glGetError());
				break;
			case ETT_FRAMEBUFFER_RGBA16:
				glGenRenderbuffers(1, &m_rbo);
				glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture->GetTextureID(), 0);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
				assert(GL_NO_ERROR == glGetError());
				break;
			default:
				result = false;
				printf("Invalid texture type in creating frame buffer.\n");
				assert(result);
				return result;
				break;
			}

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			if (!(result = (status == GL_FRAMEBUFFER_COMPLETE)))
			{
				printf("Frame buffer error in initializing: %i.\n", status);
				assert(result);
				return result;
			}

			// cleanup frame buffer, go to previous buffer
			glBindFramebuffer(GL_FRAMEBUFFER, m_prevFbo);
			m_prevFbo = 0;
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
		else {
			printf("Initialize frame buffer error, can not create frame buffer without a texture id\n");
			result = false;
		}
		return result;
	}

	Graphics::cFrameBuffer& cFrameBuffer::operator=(const cFrameBuffer& i_other)
	{
		m_fbo = i_other.m_fbo;
		m_rbo = i_other.m_rbo;
		m_prevFbo = i_other.m_prevFbo;
		m_renderToTexture = i_other.m_renderToTexture;
		m_width = i_other.m_width;
		m_height = i_other.m_height;

		return *this;
	}

	void cFrameBuffer::Write()
	{
		// Change view port size first
		Application::cApplication* _app = Application::GetCurrentApplication();
		if (_app) { _app->GetCurrentWindow()->SetViewportSize(m_width, m_height); }

		// right now, it will write current buffer to this frame buffer 
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_prevFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		if (m_rbo > 0 && m_rbo < static_cast<GLuint>(-1))
			glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);

		assert(GL_NO_ERROR == glGetError());
	}

	void cFrameBuffer::UnWrite()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_prevFbo);
		m_prevFbo = 0;
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// reset window size
		Application::cApplication* _app = Application::GetCurrentApplication();
		if (_app) { _app->ResetWindowSize(); }

		assert(GL_NO_ERROR == glGetError());
	}

	void cFrameBuffer::Read(GLenum i_textureID)
	{
		cTexture* _texture = cTexture::s_manager.Get(m_renderToTexture);
		if (_texture) {
			_texture->UseTexture(i_textureID);
		}
	}

	void cFrameBuffer::CleanUp()
	{
		if (m_fbo) {
			glDeleteFramebuffers(1, &m_fbo);
			m_fbo = 0;
			assert(GL_NO_ERROR == glGetError());
		}
		if (m_rbo)
		{
			glDeleteRenderbuffers(1, &m_rbo);
			m_rbo = 0;
			assert(GL_NO_ERROR == glGetError());
		}

		cTexture::s_manager.Release(m_renderToTexture);
	}

	bool cFrameBuffer::IsValid() const
	{
		return (m_fbo != 0);
	}

}
