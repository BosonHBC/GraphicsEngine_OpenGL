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
	void Update(Graphics::cEffect* const i_effect);
	void CleanUp();
	
	/** Setters */
	void SetModel(const std::string i_modelPath);
	void SetMaterial(Graphics::cMaterial* const i_mat) { m_material = i_mat; };

	/** Getters*/
	cTransform* Transform() const { 
		return m_transform; }
private:

	cTransform* m_transform;
	Graphics::cModel::HANDLE  m_modelHandle;
	Graphics::cMaterial* m_material;

};
