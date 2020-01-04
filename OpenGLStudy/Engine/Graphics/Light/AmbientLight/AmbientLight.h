#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {
	class cAmbientLight : public cGenLight
	{
	public:
		cAmbientLight()
			: cGenLight()
		{}

		cAmbientLight(GLfloat i_diffuseIntensity, GLfloat i_specularIntensity, glm::vec3 i_color)
			: cGenLight(i_diffuseIntensity, i_specularIntensity,	i_color)
		{}

		~cAmbientLight();

		/** Setup uniform id*/
		void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;

		/** overriding virtual functions*/
		void Illuminate();

	};

}