#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {
	class cAmbientLight : public cGenLight
	{
	public:
		cAmbientLight();
		~cAmbientLight();

		/** Setup uniform id*/
		void SetupLight(GLfloat i_intensity, glm::vec3 i_color, const GLuint& i_programID) override;

		/** overriding virtual functions*/
		void Illuminate();

	};

}