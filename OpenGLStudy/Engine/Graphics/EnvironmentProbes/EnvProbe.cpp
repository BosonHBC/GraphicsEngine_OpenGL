#include "Cores/Core.h"
#include "EnvProbe.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
#include "Graphics/Texture/Texture.h"
namespace Graphics
{    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions

	bool cEnvProbe::Initialize(GLfloat i_range, GLuint i_width, GLuint i_height, const glm::vec3& i_initialLocation /*= glm::vec3(0)*/)
	{
		auto result = true;
		m_range = i_range; m_width = i_width; m_height = i_height;
		m_transform.SetTransform(i_initialLocation, glm::quat(1, 0, 0, 0), glm::vec3(1, 1, 1));
		result = m_frameBuffer.Initialize(m_width, m_height, ETT_FRAMEBUFFER_HDR_CUBEMAP);
		return result;
	}

	// ----------------------------------------------------------------------------------------------

	bool cEnvProbe::CleanUp()
	{
		m_frameBuffer.~cFrameBuffer();
		return true;
	}

	void cEnvProbe::StartCapture()
	{
		m_frameBuffer.Write();
	}

	void cEnvProbe::StopCapture()
	{
		m_frameBuffer.UnWrite();
	}

	GLuint cEnvProbe::GetCubemapTextureID() const
	{
		cTexture* _cubemapTex = cTexture::s_manager.Get(m_frameBuffer.GetTextureHandle());
		if (_cubemapTex)
		{
			return _cubemapTex->GetTextureID();
		}
	}

	glm::mat4 cEnvProbe::GetViewMat4(GLuint i_face) const
	{
		glm::mat4 _captureViews[6] =
		{
			/*
						glm::lookAt(m_transform.Position(), m_transform.Position() +  glm::vec3(1.0f,  0.0f,  0.0f), -cTransform::WorldUp),
						glm::lookAt(m_transform.Position(),m_transform.Position() + glm::vec3(-1.0f,  0.0f,  0.0f), -cTransform::WorldUp),
						glm::lookAt(m_transform.Position(),m_transform.Position() + glm::vec3(0.0f,  1.0f,  0.0f), cTransform::WorldForward),
						glm::lookAt(m_transform.Position(),m_transform.Position() + glm::vec3(0.0f,  -1.0f,  0.0f), -cTransform::WorldForward),
						glm::lookAt(m_transform.Position(),m_transform.Position() + glm::vec3( 0.0f,  0.0f,  1.0f), -cTransform::WorldUp),
						glm::lookAt(m_transform.Position(),m_transform.Position() + glm::vec3( 0.0f,  0.0f,  -1.0f), -cTransform::WorldUp),*/
					glm::lookAt(m_transform.Position(), m_transform.Position() + glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
					glm::lookAt(m_transform.Position(), m_transform.Position() + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
					glm::lookAt(m_transform.Position(), m_transform.Position() + glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f,  0.0f,  1.0f)),
					glm::lookAt(m_transform.Position(), m_transform.Position() + glm::vec3(0.0f,  -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
					glm::lookAt(m_transform.Position(), m_transform.Position() + glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
					glm::lookAt(m_transform.Position(), m_transform.Position() + glm::vec3(0.0f,  0.0f,  -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		return _captureViews[i_face];
	}



}
