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
	m_modelHandle.CleanUp();

}

void cActor::SetModel(std::string i_modelPath)
{
	m_modelHandle = Graphics::cModel(i_modelPath);
}
