#pragma once
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "Color/Color.h"
#include "Cores/Core.h"
#include "Transform/Transform.h"

namespace Graphics {
	// An Interface for all kinds of lights
	class cGenLight
	{
	public:
		/** Constructors and destructor */
		cGenLight();
		cGenLight(Color i_color);
		virtual ~cGenLight();

		/**Usage function*/
		virtual void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0);
		cTransform* Transform() const { return m_transform; }

		/** Pure virtual functions*/
		virtual void Illuminate() = 0;

	protected:
		Color m_color;
		GLuint m_colorID;
		// record the index of this light
		GLuint m_lightIndex;

		cTransform* m_transform;
	};

}
