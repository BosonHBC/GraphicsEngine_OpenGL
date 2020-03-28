#include "Actor.h"
#include "Math/Transform/Transform.h"
#include "glm/gtc/type_ptr.hpp"

#include "Engine/Graphics/Model/Model.h"
#include "Engine/Graphics/Effect/Effect.h"
#include "Engine/Graphics/Material/Material.h"


void cActor::Initialize()
{

}


void cActor::CleanUp()
{
	// Release the handle
	Graphics::cModel::s_manager.Release(m_modelHandle);

}

void cActor::SetModel(std::string i_modelPath)
{
	if (!Graphics::cModel::s_manager.Load(i_modelPath, m_modelHandle)) {
		// TODO: print fail to load model
	}
}
