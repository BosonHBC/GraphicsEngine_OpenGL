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
		bool result = true;
		switch (m_type)
		{
		case Graphics::UBT_Frame:
			m_size = UniformBufferFormats::sFrame::Size;
			break;
		case Graphics::UBT_Drawcall:
			m_size = UniformBufferFormats::sDrawCall::Size;
			break;
		case Graphics::UBT_Invalid:
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
		if (m_bufferID == 0) {
			printf("Error: Invalid buffer id. Should not bind it to shader.");
			return;
		}

		// The buffer data index in the shader should be the same as how eUniformBufferType defines
		glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(m_type), m_bufferID);
		const auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			printf("OpenGL failed to bind the uniform buffer %u.",	 m_bufferID);
			return;
		}
	}

	void cUniformBuffer::Update(const void* const i_data)
	{
		if (m_bufferID == 0) {
			printf("Error: Invalid buffer id. Should not bind it to shader.");
			return;
		}

		// Activate the uniform buffer
		glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);

		// Copy the updated memory to the GPU
		GLintptr updateAtTheBeginning = 0;
		glBufferSubData(GL_UNIFORM_BUFFER, updateAtTheBeginning, static_cast<GLsizeiptr>(m_size), i_data);
		const auto errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			printf("OpenGL failed to update data of uniform buffer %u.",m_bufferID);
			return;
		}
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	bool cUniformBuffer::CleanUp()
	{
		m_type = UBT_Invalid;
		m_size = 0;
		if (m_bufferID != 0)
		{
			glDeleteBuffers(1, &m_bufferID);
			const auto errorCode = glGetError();
			m_bufferID = 0;
			if (errorCode != GL_NO_ERROR)
			{
				printf("OpenGL failed to delete the constant buffer.");
				return false;
			}
		}
		return true;
	}

}