#include "GeometryBuffer.h"

namespace Graphics
{

	bool cGBuffer::Initialize(GLuint i_width, GLuint i_height)
	{
		std::lock_guard<std::mutex> autoLock(m_mutex);
		auto result = true;
		m_width = i_width; m_height = i_height;
		// record the previous frame buffer object
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_prevFbo);
		// Generate another frame buffer
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		assert(GL_NO_ERROR == glGetError());
		// Bind color_attachment_0 to with Albedo&Metallic
		std::string key = "GBuffer_AlbedoMetallic";
		{
			cTexture::s_manager.Load(key, m_renderToTexture, ETT_FRAMEBUFFER_RGBA8, m_width, m_height);
			cTexture* _gAlbedoMetallic = cTexture::s_manager.Get(m_renderToTexture);
			const GLuint _textureID = _gAlbedoMetallic->GetTextureID();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureID, 0);
		}

		// Bind color_attachment_1 to with Normal&Roughness
		{
			key = "GBuffer_NormalRoughness";
			cTexture::s_manager.Load(key, m_normalHolder, ETT_FRAMEBUFFER_RGBA16, m_width, m_height);
			cTexture* _gNormalRoughness = cTexture::s_manager.Get(m_normalHolder);
			const GLuint _textureID = _gNormalRoughness->GetTextureID();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _textureID, 0);
		}

		// Bind color_attachment_2 to with IOR
		{
			key = "GBuffer_IOR";
			cTexture::s_manager.Load(key, m_iorHolder, ETT_FRAMEBUFFER_RGBA16, m_width, m_height);
			cTexture* _gIOR = cTexture::s_manager.Get(m_iorHolder);
			const GLuint _textureID = _gIOR->GetTextureID();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _textureID, 0);
		}

		// Bind depth component
		{
			key = "GBuffer_Depth";
			cTexture::s_manager.Load(key, m_depthHolder, ETT_FRAMEBUFFER_DEPTH16, m_width, m_height);
			cTexture* _gDepth = cTexture::s_manager.Get(m_depthHolder);
			const GLuint _textureID = _gDepth->GetTextureID();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _textureID, 0);
		}

		// tell OpenGL which color attachments we'll use (of this frame buffer) for rendering 
		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);
		
		assert(GL_NO_ERROR == glGetError());
		// create and bind render buffer for depth component
		//glGenRenderbuffers(1, &m_rbo);
		//glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, i_width, i_height);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
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
		cTexture::s_manager.Release(m_iorHolder);
		cTexture::s_manager.Release(m_depthHolder);

	}

	void cGBuffer::Read(GLenum* i_textureIDs)
	{
		ReadAlbedoMetallic(i_textureIDs[0]);
		ReadNormalRoughness(i_textureIDs[1]);
		ReadIOR(i_textureIDs[2]);
		ReadDepth(i_textureIDs[3]);
	}

	void cGBuffer::ReadAlbedoMetallic(GLenum i_textureID)
	{
		cTexture* _albedoMetallic = cTexture::s_manager.Get(m_renderToTexture);
		if (_albedoMetallic) {
			_albedoMetallic->UseTexture(i_textureID);
		}
	}

	void cGBuffer::ReadNormalRoughness(GLenum i_textureID)
	{
		cTexture* _normalRoughness = cTexture::s_manager.Get(m_normalHolder);
		if (_normalRoughness) {
			_normalRoughness->UseTexture(i_textureID);
		}
	}

	void cGBuffer::ReadIOR(GLenum i_textureID)
	{
		cTexture* _ior = cTexture::s_manager.Get(m_iorHolder);
		if (_ior) {
			_ior->UseTexture(i_textureID);
		}
	}

	void cGBuffer::ReadDepth(GLenum i_textureID)
	{
		cTexture* _depth = cTexture::s_manager.Get(m_depthHolder);
		if (_depth) {
			_depth->UseTexture(i_textureID);
		}
	}

}