#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {
	class cDirectionalLight : public cGenLight
	{
	public:
		cDirectionalLight() 
			: cGenLight(), m_directionalLightTransformID(0), m_directionalShadowMapID(0)
		{}
		cDirectionalLight(Color i_color, glm::vec3 i_direction);
		virtual ~cDirectionalLight() {
			m_directionalLightTransformID = 0;
			m_directionalShadowMapID = 0;
		}
		cDirectionalLight(const cDirectionalLight& i_other) :
			cGenLight(i_other), m_directionalLightTransformID(i_other.m_directionalLightTransformID), 
			m_directionalShadowMapID(i_other.m_directionalShadowMapID) {}
		cDirectionalLight& operator =(const cDirectionalLight& i_other) 
		{
			if (&i_other != nullptr)
			{
				cGenLight::operator= (i_other);
				m_directionalLightTransformID = i_other.m_directionalLightTransformID;
				m_directionalShadowMapID = i_other.m_directionalShadowMapID;
				return *this;
			}
			else
			{
				*this = cDirectionalLight();
				return *this;
			}
		}

		/** Setup uniform id*/
		void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;

		/** overriding virtual functions*/
		void Illuminate();

		/** Shadow map related*/
		void CreateShadowMap(GLuint i_width, GLuint i_height) override;
		glm::mat4 CalculateLightTransform() const override;
		void SetLightUniformTransform() override;
		void UseShadowMap(GLuint i_textureUnit) override;
	private:
		GLuint m_directionalLightTransformID = static_cast<GLuint>(-1), m_directionalShadowMapID = static_cast<GLuint>(-1);
	};

}