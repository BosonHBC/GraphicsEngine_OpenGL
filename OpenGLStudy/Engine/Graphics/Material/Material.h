#pragma once
#include "GL/glew.h"
#include "Graphics/Color/Color.h"
namespace Graphics {
	class cTexture;
	class cMaterial
	{
	public:
		cMaterial() : m_diffuse(nullptr), m_shininess(0){}
		~cMaterial() { CleanUp(); };

		void UseMaterial(GLuint i_programID);
		void CleanUp();


		void SetDiffuse(const char* i_diffusePath);
		void SetShininess(GLfloat i_shine);

	private:
		cTexture* m_diffuse;
		GLfloat m_shininess;
	};

}
