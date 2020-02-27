#pragma once
#include "Graphics/Light/PointLight/PointLight.h"

namespace Graphics {
	class cSpotLight : public cPointLight
	{
	public:
		cSpotLight() : cPointLight() {}
		cSpotLight(Color i_color, const glm::vec3& i_position,
			const glm::vec3& i_direction,
			GLfloat i_edge, GLfloat i_range,
			GLfloat i_const, GLfloat i_linear, GLfloat i_quadratic);


		virtual ~cSpotLight() {};
		

		/** overriding virtual functions*/
		void Illuminate();
		void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;

	private:
		glm::vec3 m_dir;
		GLfloat m_edge, m_procEdge;
	};

}
