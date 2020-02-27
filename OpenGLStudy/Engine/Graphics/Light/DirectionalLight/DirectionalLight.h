#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {
	class cDirectionalLight : public cGenLight
	{
	public:
		cDirectionalLight() 
			: cGenLight()
		{}
		cDirectionalLight(Color i_color, glm::vec3 i_direction);

		virtual ~cDirectionalLight();

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
		GLuint m_directionalLightTransformID, m_directionalShadowMapID;
	};

}