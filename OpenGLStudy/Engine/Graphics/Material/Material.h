#pragma once
#include "GL/glew.h"
#include "Graphics/Color/Color.h"
namespace Graphics {
	class cTexture;
	class cMaterial
	{
	public:
		cMaterial() {};
		~cMaterial() {};

	private:
		cTexture* m_diffuse;
		
	};

}
