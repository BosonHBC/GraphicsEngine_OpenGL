#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {
	class cAmbientLight : public cGenLight
	{
	public:
		cAmbientLight()
			: cGenLight()
		{}

		cAmbientLight(Color i_color)
			: cGenLight(i_color)
		{}
		// Rule of three
		virtual ~cAmbientLight();
		cAmbientLight(const cAmbientLight& i_other);
		cAmbientLight& operator =(const cAmbientLight& i_other) = delete;

		/** Setup uniform id*/
		void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;

		/** overriding virtual functions*/
		void Illuminate();

	};

}