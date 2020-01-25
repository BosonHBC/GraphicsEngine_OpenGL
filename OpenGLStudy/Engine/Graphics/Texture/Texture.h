#pragma once
#include "GL/glew.h"
// Download texture in texturer.com
namespace Graphics {
	class cTexture
	{
	public:
		/** Constructors and destructor */
		cTexture() : m_textureID (0), m_width(0), m_height(0), m_bitDepth(0), m_filePath("") {};
		cTexture(const char* i_filePath) : m_textureID(0), m_width(0), m_height(0), m_bitDepth(0), m_filePath(i_filePath) {};
		~cTexture();

		/** Usage function*/
		bool LoadTexture();
		bool LoadTextureA();
		void UseTexture(int i_textureLocation);
		void CleanUp();

	private:
		/** private variables*/
		GLuint m_textureID;
		int m_width, m_height, m_bitDepth;

		const char* m_filePath;
	};


}
