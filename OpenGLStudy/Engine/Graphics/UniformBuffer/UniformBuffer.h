#pragma once
#include "stdint.h"
#include "GL/glew.h"
namespace Graphics {
	// ---------------------------------
	// UniformBufferType defined all available buffer types 
	// It also defines the index of buffer in shader
	// ---------------------------------
	enum eUniformBufferType : uint8_t
	{
		UBT_Frame = 0,
		UBT_Drawcall = 1,
		UBT_Invalid = 0xffff,
	};

	// ---------------------------------
	// Uniform buffer is used to pass data to shaders. There are few uniform buffer types, and they are predefined
	// ---------------------------------
	class cUniformBuffer
	{
	public:
		// only allow this constructor
		cUniformBuffer(const eUniformBufferType i_ubt) : m_type(i_ubt) {}
		~cUniformBuffer();

		bool Initialize(const void* const i_data);
		// Bind the buffer
		void Bind();
		// Update data in GPU
		void Update(const void* const i_data);

		bool IsValid() const { m_type == UBT_Invalid; }

	private:
		// default is invalid
		eUniformBufferType m_type = UBT_Invalid;
		uint32_t m_size;
		GLuint m_bufferID;

		// Remove all default constructors
		cUniformBuffer() = delete;
		cUniformBuffer(const cUniformBuffer& i_other) = delete;
		cUniformBuffer& operator = (const cUniformBuffer& i_other) = delete;
	};
}


