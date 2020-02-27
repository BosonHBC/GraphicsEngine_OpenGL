#include "EditorCamera.h"
#include "Engine/Application/Window/WindowInput.h"
#include "glfw/glfw3.h"
#include "stdio.h"

cEditorCamera::cEditorCamera(const cEditorCamera& i_other) 
	: cCamera(i_other)
{

}

void cEditorCamera::CameraControl(sWindowInput* const i_windowInput, float i_dt)
{
	cCamera::CameraControl(i_windowInput, i_dt);
}

void cEditorCamera::MouseControl(sWindowInput* const i_windowInput, float i_dt)
{
	if (i_windowInput->IsButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {

		m_yaw += -i_windowInput->DX() * m_turnSpeed * i_dt;
		m_transform->Translate(m_transform->Forward() *i_windowInput->DY() *i_dt);
	}
	else if (i_windowInput->IsButtonDown(GLFW_MOUSE_BUTTON_RIGHT)) {
		m_yaw += -i_windowInput->DX() * m_turnSpeed * i_dt;
		m_pitch += -i_windowInput->DY() * m_turnSpeed * i_dt;
	}
	Update();
}

void cEditorCamera::Update()
{
	cCamera::Update();

}


