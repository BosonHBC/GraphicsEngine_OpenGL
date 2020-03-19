#include "Cores/Core.h"
#include "EnvProbe.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
#include "Graphics/Texture/Texture.h"

namespace Graphics
{    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions

	Graphics::cEnvProbe& cEnvProbe::operator=(const cEnvProbe& i_other)
	{
		m_position = i_other.m_position;
		m_captured = i_other.m_captured;
		m_frameBuffer = i_other.m_frameBuffer;
		m_range = i_other.m_range;
		m_width = i_other.m_width;
		m_height = i_other.m_height;
		return *this;
	}

	bool cEnvProbe::Initialize(GLfloat i_range, GLuint i_width, GLuint i_height, const ETextureType& i_textureType, const glm::vec3& i_initialLocation /*= glm::vec3(0)*/)
	{
		auto result = true;
		m_range = i_range; m_width = i_width; m_height = i_height;
		m_position = i_initialLocation;
		result = m_frameBuffer.Initialize(m_width, m_height, i_textureType);
		return result;
	}

	// ----------------------------------------------------------------------------------------------

	bool cEnvProbe::CleanUp()
	{
		m_frameBuffer.CleanUp();
		return true;
	}

	void cEnvProbe::StartCapture()
	{
		if (!m_captured) m_captured = true;
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
			glm::lookAt(m_position, m_position+ glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(m_position, m_position+ glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(m_position, m_position+ glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(m_position, m_position+ glm::vec3(0.0f,  -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(m_position, m_position+ glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(m_position, m_position+ glm::vec3(0.0f,  0.0f,  -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		return _captureViews[i_face];
	}



}
