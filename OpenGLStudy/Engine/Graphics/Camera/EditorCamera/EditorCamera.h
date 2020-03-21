#pragma once
#include "Graphics/Camera/Camera.h"
//
//	EditorCamera is the editor default camera
//

class  cEditorCamera : public cCamera
{
public:
	/** Constructors and destructor */
	cEditorCamera() :cCamera() {}
	cEditorCamera(glm::vec3 i_initialPos, GLfloat i_initialPitch = 0.0, GLfloat i_initialYaw = 0.0, GLfloat i_moveSpeed = 1.0, GLfloat i_turnSpeed = 1.0f) :
		cCamera(i_initialPos, i_initialPitch, i_initialYaw, i_moveSpeed, i_turnSpeed) {
		m_transform->SetPosition(i_initialPos);
		Update();
	}
	cEditorCamera(const cEditorCamera& i_other);
	~cEditorCamera() 
	{
	};


	/** Virtual functions */
	virtual void CameraControl(sWindowInput* const i_windowInput, float i_dt) override;
	virtual void MouseControl(sWindowInput* const i_windowInput, float i_dt) override;

protected:
	virtual void Update() override;
};


