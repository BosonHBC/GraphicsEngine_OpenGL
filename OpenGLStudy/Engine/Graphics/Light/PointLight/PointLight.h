#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {

	class cPointLight : public cGenLight
	{
	public:
		cPointLight() : m_position(glm::vec3(0,0,0)), m_const(1), m_linear(0), m_quadratic(0), cGenLight()
		{}
		cPointLight(GLfloat i_diffuseIntensity, GLfloat i_specularIntensity, Color i_color, glm::vec3 i_pos, GLfloat i_const, GLfloat i_linear, GLfloat i_quadratic):
			m_position(i_pos), m_const(i_const), m_linear(i_linear), m_quadratic(i_quadratic),
			cGenLight(i_diffuseIntensity, i_specularIntensity, i_color)
		{}
		~cPointLight() {}

		/** overriding virtual functions*/
		virtual void Illuminate() override;
		virtual void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;

		protected:
		glm::vec3 m_position;
		// for attenuation calculation: c+bx+ax^2
		GLfloat m_const, m_linear, m_quadratic;
		GLuint m_positionID, m_constID, m_linearID, m_quadraticID;
	};

}