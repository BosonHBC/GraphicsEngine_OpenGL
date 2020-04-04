#pragma once
#include "Graphics/Light/PointLight/PointLight.h"

namespace Graphics {
	class cSpotLight : public cPointLight
	{
	public:
		cSpotLight() : cPointLight() {}
		cSpotLight(Color i_color, const glm::vec3& i_position,
			const glm::vec3& i_direction,
			GLfloat i_edge,
			GLfloat i_radius);

		cSpotLight(const cSpotLight& i_other) : cPointLight(i_other), m_edge(i_other.m_edge), m_procEdge(i_other.m_procEdge)
		{}
		virtual ~cSpotLight() { m_edge = 0; m_procEdge = 0; m_lightTransformID = 0; m_lightShadowMapID = 0; }
		cSpotLight& operator =(const cSpotLight& i_other)
		{
			cPointLight::operator=(i_other);
			m_edge = i_other.m_edge;
			m_procEdge = i_other.m_procEdge;
			return *this;
		}


		/** overriding virtual functions*/
		void Illuminate();
		void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;
		void UseShadowMap(GLuint i_textureUnit) override;
		void CreateShadowMap(GLuint i_width, GLuint i_height) override;
		glm::mat4 GetViewMatrix() const;
		void SetLightUniformTransform() override;

	private:
		GLfloat m_edge, m_procEdge;
	};

}
