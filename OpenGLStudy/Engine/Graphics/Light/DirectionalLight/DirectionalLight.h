#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {
	class cDirectionalLight : public cGenLight
	{
	public:
		cDirectionalLight() 
			: m_direction(glm::vec3(0, -1, 0)), cGenLight()
		{}
		cDirectionalLight(Color i_color, glm::vec3 i_direction)
			: m_direction(i_direction), cGenLight(i_color)
		{}
		virtual ~cDirectionalLight();

		/** Setup uniform id*/
		void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;

		/** overriding virtual functions*/
		void Illuminate();

		glm::vec3 Direction(glm::vec3 i_position);

		/** Shadow map related*/
		void CreateShadowMap(GLuint i_width, GLuint i_height) override;
		glm::mat4 CalculateLightTransform() const override;
		void SetLightUniformTransform() override;
	private:
		glm::vec3 m_direction;
		GLuint m_directionID;
		GLuint m_directionalLightTransformID, m_directionalShadowMapID;
	};

}