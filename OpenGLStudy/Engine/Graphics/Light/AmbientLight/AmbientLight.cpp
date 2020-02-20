#include "AmbientLight.h"
#include "Graphics/Graphics.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
namespace Graphics {

	cAmbientLight::cAmbientLight(const cAmbientLight& i_other)
		:cGenLight(i_other)
	{
	}

	cAmbientLight& cAmbientLight::operator=(const cAmbientLight& i_other)
	{
		cGenLight::operator=(i_other);
		return *this;
	}

	cAmbientLight::~cAmbientLight()
	{

	}


	void cAmbientLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight( i_programID, i_lightIndex);

	}

	void cAmbientLight::Illuminate()
	{
		auto& gLighting = Graphics::GetGlobalLightingData();
		gLighting.ambientLight.base.color = m_color;
		gLighting.ambientLight.base.enableShadow = m_enableShadow;
	}


}