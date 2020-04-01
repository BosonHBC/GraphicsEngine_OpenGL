#include "GeometryBuffer.h"

namespace Graphics
{

	bool cGBuffer::Initialize(GLuint i_width, GLuint i_height)
	{
		auto result = true;
		m_width = i_width; m_height = i_height;
		// record the previous frame buffer object
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_prevFbo);
		// Generate another frame buffer
		glGenFramebuffers(1, &m_fbo);
		// Bind color_attachment_0 to with Albedo&Metallic
		std::string key = "GBuffer_AlbedoMetallic";
		{
			cTexture::s_manager.Load(key, m_renderToTexture, ETT_FRAMEBUFFER_RGBA8, m_width, m_height);
			cTexture* _gAlbedoMetallic = cTexture::s_manager.Get(m_renderToTexture);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gAlbedoMetallic->GetTextureID(), 0);
		}
		// Bind color_attachment_1 to with Normal&Roughness
		{
			key = "GBuffer_NormalRoughness";
			cTexture::s_manager.Load(key, m_normalHolder, ETT_FRAMEBUFFER_RGBA16, m_width, m_height);
			cTexture* _gNormalRoughness = cTexture::s_manager.Get(m_normalHolder);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _gNormalRoughness->GetTextureID(), 0);
		}

		// tell OpenGL which color attachments we'll use (of this frame buffer) for rendering 
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, attachments);

		// create and bind render buffer for depth component
		glGenRenderbuffers(1, &m_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, i_width, i_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

		// Check completeness
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
		return result;

	}

	void cGBuffer::CleanUp()
	{
		cFrameBuffer::CleanUp();

		cTexture::s_manager.Release(m_normalHolder);
	}


	void cGBuffer::Read(GLenum* i_textureIDs)
	{
		cTexture* _albedoMetallic = cTexture::s_manager.Get(m_renderToTexture);
		if (_albedoMetallic) {
			_albedoMetallic->UseTexture(i_textureIDs[0]);
		}
		cTexture* _normalRoughness = cTexture::s_manager.Get(m_renderToTexture);
		if (_normalRoughness) {
			_normalRoughness->UseTexture(i_textureIDs[0]);
		}
	}

}