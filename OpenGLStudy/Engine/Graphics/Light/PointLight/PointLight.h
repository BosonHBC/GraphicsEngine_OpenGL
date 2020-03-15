#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {

	class cPointLight : public cGenLight
	{
	public:
		cPointLight() : m_range(0), cGenLight()
		{}
		cPointLight(Color i_color, const glm::vec3& i_position,  GLfloat i_range);
		virtual ~cPointLight() { m_range = 0; }

		cPointLight(const cPointLight& i_other)
			: cGenLight(i_other), m_range(i_other.m_range),
			m_lightTransformID(i_other.m_lightTransformID),
			m_lightShadowMapID(i_other.m_lightShadowMapID), m_farPlaneID(i_other.m_farPlaneID)
		{}
		cPointLight& operator =(const cPointLight& i_other)
		{
			cGenLight::operator=(i_other);
			m_range = i_other.m_range;
			m_farPlaneID = i_other.m_farPlaneID;
			return *this;
		}

		/** overriding virtual functions*/
		void Illuminate() override;
		void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;
		void CreateShadowMap(GLuint i_width, GLuint i_height) override;
		void SetLightUniformTransform() override;
		void UseShadowMap(GLuint i_textureUnit) override;
	protected:
		// use inverse squared fall off
		GLfloat m_range;
		GLuint m_lightTransformID, m_lightShadowMapID;
	private:
		GLuint m_farPlaneID;
	};

}