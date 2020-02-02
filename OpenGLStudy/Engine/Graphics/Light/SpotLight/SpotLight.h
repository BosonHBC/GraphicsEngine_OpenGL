#pragma once
#include "Graphics/Light/PointLight/PointLight.h"

namespace Graphics {
	class cSpotLight : public cPointLight
	{
	public:
		cSpotLight() : cPointLight() {}
		cSpotLight(Color i_color, glm::vec3 i_pos, glm::vec3 i_dir,
			GLfloat i_edge,
			GLfloat i_const, GLfloat i_linear, GLfloat i_quadratic) :
			m_edge(i_edge),
			cPointLight(i_color, i_pos, i_const, i_linear, i_quadratic) 
		{
			m_dir = glm::normalize(i_dir);
			m_procEdge = cosf(glm::radians(m_edge));
		}

		virtual ~cSpotLight() {};
		
		void SetSpotLight(glm::vec3 i_pos, glm::vec3 i_dir);

		/** overriding virtual functions*/
		void Illuminate();
		void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;

	private:
		glm::vec3 m_dir;
		GLfloat m_edge, m_procEdge;

		GLuint m_directionID, m_edgeID;
	};

}
