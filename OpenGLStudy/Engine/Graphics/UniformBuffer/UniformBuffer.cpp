#include "stdio.h"
#include "UniformBuffer.h"
#include "UniformBufferFormats.h"
namespace Graphics {

	cUniformBuffer::~cUniformBuffer()
	{
		CleanUp();
	}

	bool cUniformBuffer::Initialize(const void* const i_data)
	{
		std::lock_guard<std::mutex> autoLock(m_mutex);
		bool result = true;
		if (m_initialized) return result;
		switch (m_type)
		{
		case UBT_Frame:
			m_size = sizeof(UniformBufferFormats::sFrame);
			break;
		case UBT_Drawcall:
			m_size = sizeof(UniformBufferFormats::sDrawCall);
			break;
		case UBT_BlinnPhongMaterial:
			m_size = sizeof(UniformBufferFormats::sBlinnPhongMaterial);
			break;
		case UBT_Lighting:
			m_size = sizeof(UniformBufferFormats::sLighting);
			break;
		case UBT_ClipPlane:
			m_size = sizeof(UniformBufferFormats::sClipPlane);
			break;
		case UBT_PBRMR:
			m_size = sizeof(UniformBufferFormats::sPBRMRMaterial);
			break;
		case UBT_EnvCaptureWeight:
			m_size = sizeof(UniformBufferFormats::sEnvCaptureWeight);
			break;
		case UBT_Invalid:
			result = false;
			return result;
			break;
		default:
			result = false;
			return result;
			break;
		}
		if (!(result = (m_size > 0))) {
			printf("Error: Invalid buffer size initialization.");
			return result;
		}

		// Generate uniform buffer here
		{
			// Generate 1 buffer
			glGenBuffers(1, &m_bufferID);
			const auto errorCode = glGetError();
			if (errorCode == GL_NO_ERROR)
			{
				glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					result = false;
					printf("OpenGL failed to bind the new uniform buffer %u.", m_bufferID);
					return result;
				}
			}
			else
			{
				result = false;
				printf("OpenGL failed to get an unused uniform buffer ID.");
				return result;
			}
		}
		// Copy the data to the buffer
		{
			glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(m_size),
				reinterpret_cast<const GLvoid*>(i_data), GL_DYNAMIC_DRAW);
			const auto errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				result = false;
				printf("OpenGL failed to allocate the new uniform buffer %u.", m_bufferID);
				return result;
			}
		}

		return result;
	}

	void cUniformBuffer::Bind()
	{
		std::lock_guard<std::mutex> autoLock(m_mutex);
		if (m_initialized) return;

		if (m_bufferID == 0) {
			printf("Error: Invalid buffer id. Should not bind it to shader.\n");
			return;
		}

		// The buffer data index in the shader should be the same as how eUniformBufferType defines
		glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(m_type), m_bufferID);
		const auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			printf("OpenGL failed to bind the uniform buffer %u.\n",	 m_bufferID);
			return;
		}

		if (!m_initialized) m_initialized = true;
	}

	void cUniformBuffer::Update(const void* const i_data)
	{
		std::lock_guard<std::mutex> autoLock(m_mutex);
		if (m_bufferID == static_cast<GLuint>(-1)) {
			printf("Error: Invalid buffer id. Should not bind it to shader.");
			return;
		}

		// Activate the uniform buffer
		glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);
		auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			printf("OpenGL failed to bind uniform buffer %u.\n", m_bufferID);
		}
		// Copy the updated memory to the GPU
		GLintptr updateAtTheBeginning = 0;
		glBufferSubData(GL_UNIFORM_BUFFER, updateAtTheBeginning, static_cast<GLsizeiptr>(m_size), i_data);
		
		if (errorCode != GL_NO_ERROR)
		{
			printf("OpenGL failed to update data of uniform buffer %u.\n",m_bufferID);
		}
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void cUniformBuffer::UpdatePartially(const void* const i_data, GLintptr i_offset, GLintptr i_size)
	{
		std::lock_guard<std::mutex> autoLock(m_mutex);
		if (m_bufferID == 0) {
			printf("Error: Invalid buffer id. Should not bind it to shader.");
			return;
		}
 		if (static_cast<uint32_t>(i_offset) >= m_size || static_cast<uint32_t>(i_size) > m_size)
		{
			printf("Error: Invalid data size of update request with id: %d.", m_bufferID);
			return;
		}

		// Activate the uniform buffer
		glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);
		auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			printf("OpenGL failed to bind uniform buffer %u.\n", m_bufferID);
		}

		// Update data partially
		{
			glBufferSubData(GL_UNIFORM_BUFFER, i_offset, i_size, i_data);
		}

		if (errorCode != GL_NO_ERROR)
		{
			printf("OpenGL failed to update data of uniform buffer %u.\n", m_bufferID);
		}
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	bool cUniformBuffer::CleanUp()
	{
		if (m_bufferID != 0)
		{
			glDeleteBuffers(1, &m_bufferID);
			const auto errorCode = glGetError();
			m_bufferID = 0;
			if (errorCode != GL_NO_ERROR)
			{
				printf("OpenGL failed to delete the constant buffer with type: %d.", m_type);
				return false;
			}
			m_type = UBT_Invalid;
			m_size = 0;
		}
		return true;
	}

}