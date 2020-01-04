#pragma once
#include "GL/glew.h"
#include "glm/glm.hpp"

namespace Graphics {
	// An Interface for all kinds of lights
	class cGenLight
	{
	public:
		/** Constructors and destructor */
		cGenLight() : m_diffuseIntensity(1), m_specularIntensity(1), m_color(glm::vec3(1.0, 1.0, 1.0)), m_colorID(0), m_diffuseIntensityID(0), m_specularIntensityID(0)
		{};
		cGenLight(GLfloat i_diffuseIntensity, GLfloat i_specularIntensity, glm::vec3 i_color)
			: m_diffuseIntensity(i_diffuseIntensity), m_specularIntensity(i_specularIntensity), m_color(i_color) 
		{}
		virtual ~cGenLight();

		/**Usage function*/
		virtual void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0);

		/** Pure virtual functions*/
		virtual void Illuminate() = 0;

	protected:
		GLfloat m_diffuseIntensity, m_specularIntensity;
		glm::vec3 m_color;
		GLuint m_colorID, m_diffuseIntensityID, m_specularIntensityID;
		// record the index of this light
		GLuint m_lightIndex;
	};

}
