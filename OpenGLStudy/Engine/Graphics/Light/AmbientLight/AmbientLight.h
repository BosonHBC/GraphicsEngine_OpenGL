#pragma once
#include "Graphics/Light/Light.h"
#include "GL/glew.h"
#include "glm/glm.hpp"

namespace Graphics {
	class cAmbientLight : public cGenLight
	{
	public:
		cAmbientLight(): m_ambientIntensityID(0), m_ambientColorID(0)
		{
		}
		~cAmbientLight();

		/** Setup uniform id*/
		void SetupLight(GLfloat i_intensity, glm::vec3 i_color, const GLuint& i_programID) override;

		/** overriding virtual functions*/
		void Illuminate();
		glm::vec3 Direction(glm::vec3 i_position);

	private:
		GLuint m_ambientIntensityID, m_ambientColorID;
	};

}