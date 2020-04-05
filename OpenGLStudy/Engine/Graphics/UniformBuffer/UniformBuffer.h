#pragma once
#include "stdint.h"
#include "GL/glew.h"
#include <mutex>
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
		UBT_PBRMR = 5,
		UBT_EnvCaptureWeight = 6,
		UBT_PostProcessing = 7,
		UBT_SSAO = 8,
		UBT_Invalid = 0xff,
	};

	// ---------------------------------
	// Uniform buffer is used to pass data to shaders. There are few uniform buffer types, and they are predefined
	// ---------------------------------
	class cUniformBuffer
	{
	public:
		// only allow this constructor
		cUniformBuffer() :m_size(0), m_bufferID(static_cast<GLuint>(-1)), m_initialized(false), m_type(UBT_Invalid) {}
		cUniformBuffer(const eUniformBufferType i_ubt) : m_type(i_ubt), m_initialized(false){}
		cUniformBuffer(const cUniformBuffer& i_other) : m_size(i_other.m_size), m_bufferID(i_other.m_bufferID), m_initialized(i_other.m_initialized), m_type(i_other.m_type)
		{}
		cUniformBuffer& operator = (const cUniformBuffer& i_other) = delete;
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

		uint32_t m_size = 0;
		GLuint m_bufferID = static_cast<GLuint>(-1);
		// prevent repeated initialization
		bool m_initialized = false;
		eUniformBufferType m_type = UBT_Invalid; // default is invalid

		std::mutex m_mutex;

	};
}


