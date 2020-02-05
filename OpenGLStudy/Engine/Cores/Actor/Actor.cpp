#include "Actor.h"
#include "Math/Transform/Transform.h"
#include "glm/gtc/type_ptr.hpp"

#include "Model/Model.h"
#include "Effect/Effect.h"
#include "Material/Material.h"

void cActor::Initialize()
{
	m_transform = new cTransform();
}

void cActor::Update(Graphics::cEffect* const i_effect)
{
	// TransformUpdate
	//---------------------------------
	m_transform->Update();
	glUniformMatrix4fv(i_effect->GetModelMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(m_transform->M()));
	// fix non-uniform scale
	glUniformMatrix4fv(i_effect->GetNormalMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(glm::transpose(m_transform->MInv())));

	// Rendering Update
	//---------------------------------
	if (m_material) {
		m_material->UseMaterial(i_effect->GetProgramID());
	}
	if (m_model) {
		m_model->Render();
	}
}

void cActor::CleanUp()
{
	safe_delete(m_transform);
	
	// TODO: Object Pool.....
	// Here should let object pool to delete, but for the simplicity sake, delete here right now
	safe_delete(m_model);
	safe_delete(m_material);
}
