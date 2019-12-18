#pragma once
#include "GL/glew.h"
#include "glm/glm.hpp"

namespace Graphics {
	// An Interface for all kinds of lights
	class cGenLight
	{
	public:
		/** Constructors and destructor */
		cGenLight() : m_intensity(1), m_color(glm::vec3(1.0, 1.0, 1.0))
		{
		};
		~cGenLight();

		/**Usage function*/
		virtual void SetupLight(GLfloat i_intensity, glm::vec3 i_color,const GLuint& i_programID);

		/** Pure virtual functions*/
		virtual glm::vec3 Direction(glm::vec3 i_position) = 0;
		virtual void Illuminate() = 0;

	protected:
		GLfloat m_intensity;
		glm::vec3 m_color;
	};

}
