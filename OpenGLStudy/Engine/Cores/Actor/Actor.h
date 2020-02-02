#pragma once
#include "Engine/Cores/Core.h"

/** Forward deceleration*/
//----------------------------------------------
class cTransform;
namespace Graphics {
	class cModel;
	class cMaterial;
	class cEffect;
}
//----------------------------------------------
class cActor
{
public:
	cActor() {};
	~cActor() { CleanUp(); }

	void Initialize();
	void Update(Graphics::cEffect* const i_effect);
	void CleanUp();
	
	/** Setters */
	void SetModel(Graphics::cModel* const i_model) { m_model = i_model; };
	void SetMaterial(Graphics::cMaterial* const i_mat) { m_material = i_mat; };

	/** Getters*/
	cTransform* Transform() const { return m_transform; }
private:

	cTransform* m_transform;
	Graphics::cModel* m_model;
	Graphics::cMaterial* m_material;

};
