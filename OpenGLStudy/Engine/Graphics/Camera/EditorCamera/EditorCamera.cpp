#include "EditorCamera.h"
#include "Engine/Application/Window/WindowInput.h"
#include "glfw/glfw3.h"
#include "stdio.h"
#include "Application/imgui/imgui.h"
#include "Cores/Core.h"
cEditorCamera::cEditorCamera(const cEditorCamera& i_other)
	: cCamera(i_other)
{

}

cEditorCamera& cEditorCamera::operator=(const cEditorCamera& i_other)
{
	cCamera::operator=(i_other);
	return *this;
}

void cEditorCamera::CameraControl(sWindowInput* const i_windowInput, float i_dt)
{
	cCamera::CameraControl(i_windowInput, i_dt);
}

void cEditorCamera::MouseControl(sWindowInput* const i_windowInput, float i_dt)
{
	if (ImGui::GetIO().WantCaptureMouse) return;

	//m_yaw += -i_windowInput->DX() * m_turnSpeed * i_dt;
	if (!IsFloatZero(ImGui::GetIO().MouseWheel))
		Transform.Translate(Transform.Forward() * ImGui::GetIO().MouseWheel * 300.f *i_dt);

	if (ImGui::IsMouseDown(1)) {
		
		m_yaw += -ImGui::GetIO().MouseDelta.x * m_turnSpeed * i_dt;
		m_pitch += ImGui::GetIO().MouseDelta.y * m_turnSpeed * i_dt;
	}
	Update();
}

void cEditorCamera::Update()
{
	cCamera::Update();

}


