#pragma once
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "Color/Color.h"

namespace Graphics {
	// An Interface for all kinds of lights
	class cGenLight
	{
	public:
		/** Constructors and destructor */
		cGenLight() : m_color(Color::White()), m_colorID(0)
		{};
		cGenLight(Color i_color)
			:m_color(i_color) 
		{}
		virtual ~cGenLight();

		/**Usage function*/
		virtual void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0);

		/** Pure virtual functions*/
		virtual void Illuminate() = 0;

	protected:
		Color m_color;
		GLuint m_colorID;
		// record the index of this light
		GLuint m_lightIndex;
	};

}
