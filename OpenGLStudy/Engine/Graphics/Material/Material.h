#pragma once
#include "GL/glew.h"
#include "Graphics/Color/Color.h"
namespace Graphics {
	class cTexture;
	class cMaterial
	{
	public:
		cMaterial() 
			: m_diffuseTexture(nullptr), m_shininess(0), m_diffuseIntensity(Color::White()), m_specularIntensity(Color::White())
		{}
		cMaterial(Color i_diffuseIntensity, Color i_specularIntensity)
			:m_diffuseTexture(nullptr), m_diffuseIntensity(i_diffuseIntensity), m_specularIntensity(i_diffuseIntensity)
		{}
		~cMaterial() { CleanUp(); };

		void UseMaterial( GLuint i_programID);
		void CleanUp();

		/** Setters */
		void SetDiffuse(const char* i_diffusePath);
		void SetShininess(GLuint i_programID, GLfloat i_shine);
		void SetDiffuseIntensity(GLuint i_programID, Color i_diffuseIntensity);
		void SetSpecularIntensity(GLuint i_programID, Color i_diffuseIntensity);

		protected:
		cTexture* m_diffuseTexture;
		Color m_diffuseIntensity, m_specularIntensity;
		GLfloat m_shininess;

		GLuint m_shininessID, m_diffuseIntensityID, m_specularIntensityID;
	};

}
