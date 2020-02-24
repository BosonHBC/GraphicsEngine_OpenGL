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
		UBT_BlinnPhongMaterial = 2,
		UBT_Lighting = 3,
		UBT_ClipPlane = 4,
		UBT_Invalid = 0xff,
	};

	// ---------------------------------
	// Uniform buffer is used to pass data to shaders. There are few uniform buffer types, and they are predefined
	// ---------------------------------
	class cUniformBuffer
	{
	public:
		// only allow this constructor
		cUniformBuffer(const eUniformBufferType i_ubt) : m_type(i_ubt), m_initialized(false){}
		~cUniformBuffer();

		// Create the uniform buffer
		bool Initialize(const void* const i_data);
		
		// Bind the buffer
		void Bind();

		// Update all data in GPU
		void Update(const void* const i_data);

		// Update data partially in the GPU, use this carefully !!!
		void UpdatePartially(const void* const i_data, GLintptr i_offset, GLintptr i_size);

		bool CleanUp();
		bool IsValid() const { return m_type == UBT_Invalid; }

	private:
		// default is invalid
		eUniformBufferType m_type = UBT_Invalid;
		uint32_t m_size;
		GLuint m_bufferID;
		// prevent repeated initialization
		bool m_initialized;

		// Remove all default constructors
		cUniformBuffer() = delete;
		cUniformBuffer(const cUniformBuffer& i_other) = delete;
		cUniformBuffer& operator = (const cUniformBuffer& i_other) = delete;
	};
}


