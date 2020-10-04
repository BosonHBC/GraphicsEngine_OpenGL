#pragma once

#include "GL/glew.h"
#include "Graphics/Texture/Texture.h"
#include "Math/Shape/Rect.h"
#include <mutex>
namespace Graphics {
	class cFrameBuffer
	{
	public:
		cFrameBuffer()
			: m_fbo(0), m_rbo(0), m_prevFbo(0), m_width(0), m_height(0) {}

		bool Initialize(GLuint i_width, GLuint i_height, ETextureType i_textureType);
		cFrameBuffer(const cFrameBuffer& i_other) = default;
		cFrameBuffer& operator = (const cFrameBuffer& i_other) = default;

		// Write current buffer data to this frame buffer
		void Write(const std::function<void()>& captureFunction);
		// Write current buffer to this frame buffer with offset
		void WriteSubArea(const std::function<void()>& captureFunction, const sRect& i_offsetArea);
		// Switch back to original frame buffer
		void UnWrite();
		// Use the texture loaded from the frame buffer
		void Read(GLenum i_textureID);

		 ~cFrameBuffer() {}

		// Frame buffer must to clean up by the user
		void CleanUp();

		/** Getters */
		bool IsValid() const;
		GLuint GetWidth() const { return m_width; }
		GLuint GetHeight() const { return m_height; }
		cTexture::HANDLE GetTextureHandle() const { return m_renderToTexture; }
		GLuint fbo() const { return m_fbo; }
		GLuint rbo() const { return m_rbo; }
	protected:
		// fbo: frame buffer object
		GLuint m_fbo = static_cast<GLuint>(-1); // this is necessary
		GLuint m_rbo = static_cast<GLuint>(-1); // alternative render buffer object if needed.
		GLint m_prevFbo = static_cast<GLuint>(-1); // stores the previous frame buffer object.
		cTexture::HANDLE m_renderToTexture;

		// generated map should has same size as the window
		GLuint m_width = 0, m_height = 0;

		std::mutex m_mutex;
	};
}
