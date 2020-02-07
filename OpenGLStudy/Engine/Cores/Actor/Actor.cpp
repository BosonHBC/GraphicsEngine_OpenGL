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

void cActor::UpdateUniformVariables(Graphics::cEffect* const i_effect)
{
	Graphics::cModel* _modelInst = Graphics::cModel::s_manager.Get(m_modelHandle);
	if (_modelInst) {
		_modelInst->UpdateUniformVariables(i_effect->GetProgramID());
	}
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
		m_material->UseMaterial();
	}
	Graphics::cModel* _model = Graphics::cModel::s_manager.Get(m_modelHandle);
	if (_model) {
		_model->Render();
	}
}

void cActor::CleanUp()
{
	safe_delete(m_transform);

	// TODO: Handle
	// Here should let object pool to delete, but for the simplicity sake, delete here right now
	safe_delete(m_material);

	// Release the handle
	Graphics::cModel::s_manager.Release(m_modelHandle);

}

void cActor::SetModel(std::string i_modelPath)
{
	if (!Graphics::cModel::s_manager.Load(i_modelPath, m_modelHandle)) {
		// TODO: print fail to load model
	}
}
