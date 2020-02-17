#include "cFrameBuffer.h"
#include "stdio.h"
#include <string>
namespace Graphics {

	bool cFrameBuffer::Initialize(GLuint i_width, GLuint i_height, ETextureType i_textureType)
	{
		auto result = true;
		m_width = i_height; m_height = i_height;
		const GLuint mipMapLevel = 0;

		// Generate another frame buffer
		glGenFramebuffers(1, &m_fbo);
		std::string key = "FB_" +std::to_string(m_fbo) + "_ETT_" + std::to_string(i_textureType);

		// Generate render_to_texture texture
		cTexture::s_manager.Load(key, m_renderToTexture, i_textureType, m_width, m_height);
		cTexture* _texture = cTexture::s_manager.Get(m_renderToTexture);
		if (_texture) {

			// bind the frame buffer, it can be read / draw. GL_DRAW_FRAMEBUFFER / GL_READ_FRAMEBUFFER
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

			// write depth map
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _texture->GetTextureID(), mipMapLevel);

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
		return (m_fbo!=0);
	}

}
