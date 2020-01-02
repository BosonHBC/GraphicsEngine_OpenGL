#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {
	class cDirectionalLight : public cGenLight
	{
	public:
		cDirectionalLight();
		~cDirectionalLight();

		/** Setup uniform id*/
		void SetupLight(GLfloat i_intensity, glm::vec3 i_color, const GLuint& i_programID) override;
		void SetupLightDirection(glm::vec3 i_direction);

		/** overriding virtual functions*/
		void Illuminate();
		glm::vec3 Direction(glm::vec3 i_position);

	private:
		glm::vec3 m_direction;
		GLuint m_directionID;
	};

}