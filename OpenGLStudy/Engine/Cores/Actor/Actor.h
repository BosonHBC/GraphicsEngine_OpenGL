#pragma once
#include "Engine/Cores/Core.h"
#include <string>

#include "Engine/Graphics/Model/Model.h"
/** Forward deceleration*/
//----------------------------------------------
class cTransform;

namespace Graphics {
	class cMaterial;
	class cEffect;
}
//----------------------------------------------
class cActor
{
public:
	cActor() {};
	// Rule of three
	virtual ~cActor() { CleanUp(); }

	void Initialize();
	void UpdateUniformVariables(Graphics::cEffect* const i_effect);
	void Update(Graphics::cEffect* const i_effect);
	void CleanUp();
	
	/** Setters */
	void SetModel(const std::string i_modelPath);

	/** Getters*/
	cTransform* Transform() const { 
		return m_transform; }
	Graphics::cModel::HANDLE GetModelHandle() const { return m_modelHandle; }
private:

	cTransform* m_transform;
	Graphics::cModel::HANDLE  m_modelHandle;


};
