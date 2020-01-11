#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {
	class cDirectionalLight : public cGenLight
	{
	public:
		cDirectionalLight() 
			: m_direction(glm::vec3(0, -1, 0)), cGenLight()
		{}
		cDirectionalLight(GLfloat i_diffuseIntensity, GLfloat i_specularIntensity, Color i_color, glm::vec3 i_direction)
			: m_direction(i_direction), cGenLight(i_diffuseIntensity, i_specularIntensity, i_color)
		{}
		~cDirectionalLight();

		/** Setup uniform id*/
		void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;

		/** overriding virtual functions*/
		void Illuminate();

		glm::vec3 Direction(glm::vec3 i_position);

	private:
		glm::vec3 m_direction;
		GLuint m_directionID;
	};

}