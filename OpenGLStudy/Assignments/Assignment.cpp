#include "Assignment.h"
#include "Application/Window/Window.h"
#include "Application/Window/WindowInput.h"
#include "Constants/Constants.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Graphics/Graphics.h"
#include "Time/Time.h"

#include "Graphics/Camera/EditorCamera/EditorCamera.h"
#include "Graphics/Graphics.h"
#include "Material/Material.h"
#include "Actor/Actor.h"
#include "Transform/Transform.h"
#include "Engine/Graphics/Model/Model.h"
#include "Material/Blinn/MatBlinn.h"
#include "Material/Cubemap/MatCubemap.h"
#include "Graphics/Texture/Texture.h"

#include <map>

bool Assignment::Initialize(GLuint i_width, GLuint i_height, const char* i_windowName /*= "Default Window"*/)
{
	auto result = true;
	if (!(result = cApplication::Initialize(i_width, i_height, i_windowName))) {
		assert(false);
		// TODO: LogError
		printf("Failed to initialize Application!");
		return false;
	}

	CreateActor();
	CreateCamera();
	CreateLight();

	Graphics::cMatCubemap* _cubemapMat = dynamic_cast<Graphics::cMatCubemap*>(Graphics::cModel::s_manager.Get(m_cubemap->GetModelHandle())->GetMaterialAt());

	Graphics::cMatBlinn* _wallMat = dynamic_cast<Graphics::cMatBlinn*>(Graphics::cModel::s_manager.Get(m_mirror->GetModelHandle())->GetMaterialAt());
	auto cameraViewTextureHandle = Graphics::GetCameraCaptureFrameBuffer()->GetTextureHandle();
	//_wallMat->UpdateDiffuseTexture(cameraViewTextureHandle);
	//_wallMat->UpdateSpecularTexture(cameraViewTextureHandle);
	_wallMat->UpdateReflectionTexture(cameraViewTextureHandle);
	_wallMat->UpdateCubemapTexture(_cubemapMat->GetCubemapHandle());

	Graphics::cMatBlinn* _teapot2Mat = dynamic_cast<Graphics::cMatBlinn*>(Graphics::cModel::s_manager.Get(m_teapot2->GetModelHandle())->GetMaterialAt());
	_teapot2Mat->UpdateCubemapTexture(_cubemapMat->GetCubemapHandle());

	return result;
}

void Assignment::CreateActor()
{
	m_teapot = new cActor();
	m_teapot->Initialize();
	m_teapot->Transform()->SetTransform(glm::vec3(0, 0, -100), glm::quat(glm::vec3(-glm::radians(90.f), 0, 0)), glm::vec3(5, 5, 5));
	m_teapot->SetModel("Contents/models/teapot.model");
	m_teapot->UpdateUniformVariables(Graphics::GetCurrentEffect());

	m_teapot2 = new cActor();
	m_teapot2->Initialize();
	m_teapot2->Transform()->SetTransform(glm::vec3(150, 0, 0), glm::quat(glm::vec3(-glm::radians(90.f), 0, 0)), glm::vec3(3, 3, 3));
	m_teapot2->SetModel("Contents/models/teapot.model");
	m_teapot2->UpdateUniformVariables(Graphics::GetCurrentEffect());

	m_mirror = new cActor();
	m_mirror->Initialize();
	m_mirror->Transform()->SetTransform(glm::vec3(0, 0, 0), glm::quat(1, 0, 0, 0), glm::vec3(800, 1, 800));
	m_mirror->SetModel("Contents/models/wall.model");
	m_mirror->UpdateUniformVariables(Graphics::GetCurrentEffect());

	m_sphere = new cActor();
	m_sphere->Initialize();
	m_sphere->Transform()->SetTransform(glm::vec3(-100.f, 25.f, 0), glm::quat(1, 0, 0, 0), glm::vec3(2.5f, 2.5f, 2.5f));
	m_sphere->SetModel("Contents/models/sphere.model");
	m_sphere->UpdateUniformVariables(Graphics::GetCurrentEffect());

	m_cubemap = new cActor();
	m_cubemap->Initialize();
	m_cubemap->SetModel("Contents/models/cubemap.model");
	m_cubemap->UpdateUniformVariables(Graphics::GetEffectByKey("CubemapEffect"));

}

void Assignment::CreateCamera()
{
	m_editorCamera = new  cEditorCamera(glm::vec3(0, 150, 200), 30, 0, 300, 10.f);
	float _aspect = (float)(GetCurrentWindow()->GetBufferWidth()) / (float)(GetCurrentWindow()->GetBufferHeight());
	m_editorCamera->CreateProjectionMatrix(45.0f, _aspect, 1.f, 1000.0f);
	m_editorCamera->Transform()->Update();
}

void Assignment::CreateLight()
{
	Graphics::CreateAmbientLight(Color(0.1f, 0.1f, 0.1f), aLight);
	//Graphics::CreatePointLight(glm::vec3(0, 150.f, 100.f), Color(0.1, 0.2, 0.8), 0.1f, 0.003f, 0.00003f, false, pLight1);
	//Graphics::CreatePointLight(glm::vec3(-200, 100, -200), Color(0.8, 0.2, 0.2), 0.1f, 0.002f, 0.00002f, false, pLight2);
	Graphics::CreateDirectionalLight(Color(1, 1, 1), glm::vec3(-1, -1, 0), true, dLight);
	Graphics::CreateSpotLight(glm::vec3(0, 150, 0), glm::vec3(0, 1, 1), Color(0.8), 65.f, 0.1f, 0.03f, 0.0003f, true, spLight);
}

void Assignment::Run()
{
	// loop until window closed
	while (!m_shouldApplicationLoopExit)
	{
		glfwPollEvents();

		Graphics::RenderFrame();
		//dLight->Transform()->Rotate(cTransform::WorldUp, 0.01677f);
/*
		if (true)
		{
			std::vector<cTransform*> _transforms;
			cTransform _worldTransform;
			_transforms.push_back(&_worldTransform);
			_transforms.push_back(m_teapot->Transform());
			//_transforms.push_back(m_teapot2->Transform());
			//_transforms.push_back(m_sphere->Transform());
			if(pLight1)
			_transforms.push_back(pLight1->Transform());
			if (pLight2)
				_transforms.push_back(pLight2->Transform());
			if (spLight)
				_transforms.push_back(spLight->Transform());

			Graphics::SubmitTransformToBeDisplayedWithTransformGizmo(_transforms);
		}*/

		// ----------------------
		// Swap buffers
		m_window->SwapBuffers();
	}
}

void Assignment::CleanUp()
{
	safe_delete(m_editorCamera);
	safe_delete(m_teapot);
	safe_delete(m_teapot2);
	safe_delete(m_mirror);
	safe_delete(m_sphere);
	safe_delete(m_cubemap);
}

void Assignment::Tick(float second_since_lastFrame)
{
	sWindowInput* _windowInput = m_window->GetWindowInput();

	// get + handle user input events
	{
		m_editorCamera->CameraControl(_windowInput, second_since_lastFrame);

		m_editorCamera->MouseControl(_windowInput, 0.01667f);
	}
	// for recompile shader
	if (m_window->GetWindowInput()->IsKeyDown(GLFW_KEY_F6)) {
		Graphics::GetCurrentEffect()->RecompileShader(Constants::CONST_PATH_DEFAULT_VERTEXSHADER, GL_VERTEX_SHADER);
	}
	//dLight->Transform()->Rotate(-cTransform::WorldUp, 0.01677f);
	if (m_teapot) {
		m_teapot->Transform()->gRotate(glm::vec3(0, 1.f, 0), second_since_lastFrame);
	}

	cTransform* controledActor = nullptr;
	controledActor = spLight->Transform();
	//controledActor = m_sphere->Transform();
	if (controledActor) {
		if (_windowInput->IsKeyDown(GLFW_KEY_J)) {
			controledActor->Translate(-cTransform::WorldRight * 100.f * second_since_lastFrame);
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_L)) {
			controledActor->Translate(cTransform::WorldRight* 100.f  * second_since_lastFrame);
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_I)) {
			controledActor->Translate(-cTransform::WorldForward* 100.f  * second_since_lastFrame);
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_K)) {
			controledActor->Translate(cTransform::WorldForward* 100.f  * second_since_lastFrame);
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_SPACE)) {
			controledActor->Translate(cTransform::WorldUp* 100.f* second_since_lastFrame);
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
			controledActor->Translate(-cTransform::WorldUp* 100.f * second_since_lastFrame);
		}

	}

	cTransform* rotateControl = nullptr;
	if (spLight)
		rotateControl = spLight->Transform();
	//if (dLight)
		//rotateControl = dLight->Transform();
	if (rotateControl)
	{
		if (_windowInput->IsKeyDown(GLFW_KEY_LEFT)) {
			rotateControl->Rotate(cTransform::WorldUp, second_since_lastFrame);
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_RIGHT)) {
			rotateControl->Rotate(-cTransform::WorldUp, second_since_lastFrame);
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_UP)) {
			rotateControl->Rotate(cTransform::WorldRight, second_since_lastFrame);
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_DOWN)) {
			rotateControl->Rotate(-cTransform::WorldRight, second_since_lastFrame);
		}
	}
	m_teapot->Transform()->Update();
	m_teapot2->Transform()->Update();
	m_mirror->Transform()->Update();
	m_sphere->Transform()->Update();

	if (pLight1)
		pLight1->Transform()->Update();
	if (pLight2)
		pLight2->Transform()->Update();
	if (dLight)
		dLight->Transform()->Update();
	if (spLight)
		spLight->Transform()->Update();
	// Submit data
	{

		/** 1. Wait until render thread is ready for receiving new graphic data */
		Graphics::MakeApplicationThreadWaitForSwapingData(m_applicationMutex);

		/** 2. Clear the application thread data and submit new one */
		{
			// Submit geometry data
			{
				Graphics::ClearApplicationThreadData();
				// Submit data to be render
				std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>> _renderingMap;
				_renderingMap.push_back({ m_sphere->GetModelHandle(), *m_sphere->Transform() });
				_renderingMap.push_back({ m_teapot->GetModelHandle(), *m_teapot->Transform() });
				_renderingMap.push_back({ m_teapot2->GetModelHandle(), *m_teapot2->Transform() });
				_renderingMap.push_back({ m_mirror->GetModelHandle(), *m_mirror->Transform() });
				// Frame data from directional light
				if (dLight) {
					Graphics::UniformBufferFormats::sFrame _frameData_Shadow(dLight->CalculateLightTransform());
					Graphics::SubmitDataToBeRendered(_frameData_Shadow, _renderingMap, &Graphics::DirectionalShadowMap_Pass);
				}
				if (spLight) {
					Graphics::UniformBufferFormats::sFrame _frameData_Shadow(spLight->CalculateLightTransform());
					_frameData_Shadow.ViewPosition = spLight->Transform()->Position();
					Graphics::SubmitDataToBeRendered(_frameData_Shadow, _renderingMap, &Graphics::SpotLightShadowMap_Pass);
				}

				// Frame data from camera
				if (true)
				{
					cEditorCamera _mirroredCamera = *m_editorCamera;
					_mirroredCamera.MirrorAlongPlane(*m_mirror->Transform());
					Graphics::UniformBufferFormats::sFrame _frameData_Mirrored(_mirroredCamera.GetProjectionMatrix(), _mirroredCamera.GetViewMatrix());
					_frameData_Mirrored.ViewPosition = _mirroredCamera.CamLocation();
					_renderingMap.clear();
					_renderingMap.push_back({ m_teapot->GetModelHandle(), *m_teapot->Transform() });
					_renderingMap.push_back({ m_sphere->GetModelHandle(), *m_sphere->Transform() });
					_renderingMap.push_back({ m_teapot2->GetModelHandle(), *m_teapot2->Transform() });
					glm::vec4 _plane = glm::vec4(m_mirror->Transform()->Up(), m_mirror->Transform()->Position().y);
					Graphics::SubmitClipPlaneData(_plane);
					Graphics::SubmitDataToBeRendered(_frameData_Mirrored, _renderingMap, &Graphics::Render_Pass_CaptureCameraView);
				}

				_renderingMap.clear();
				Graphics::UniformBufferFormats::sFrame _frameData_Camera(m_editorCamera->GetProjectionMatrix(), m_editorCamera->GetViewMatrix());
				_frameData_Camera.ViewPosition = m_editorCamera->CamLocation();
				_renderingMap.push_back({ m_teapot->GetModelHandle(), *m_teapot->Transform() });
				_renderingMap.push_back({ m_teapot2->GetModelHandle(), *m_teapot2->Transform() });
				_renderingMap.push_back({ m_mirror->GetModelHandle(), *m_mirror->Transform() });
				_renderingMap.push_back({ m_sphere->GetModelHandle(), *m_sphere->Transform() });

				Graphics::SubmitDataToBeRendered(_frameData_Camera, _renderingMap, &Graphics::Render_Pass);

				// Cube map
				_renderingMap.clear();
				_renderingMap.push_back({ m_cubemap->GetModelHandle(), *m_cubemap->Transform() });
				Graphics::UniformBufferFormats::sFrame _frameData_Cubemap(m_editorCamera->GetProjectionMatrix(), glm::mat4(glm::mat3(m_editorCamera->GetViewMatrix())));
				Graphics::SubmitDataToBeRendered(_frameData_Camera, _renderingMap, &Graphics::CubeMap_Pass);
			}

			// Submit lighting data
			{
				std::vector<Graphics::cPointLight> _pLights;
				std::vector<Graphics::cSpotLight> _spLights;
				if (pLight1)
				{
					pLight1->UpdateLightIndex(_pLights.size());
					_pLights.push_back(*pLight1);
				}
					
				if (pLight2)
				{
					pLight2->UpdateLightIndex(_pLights.size());
					_pLights.push_back(*pLight2);
				}
					
				if (spLight)
				{
					spLight->UpdateLightIndex(_spLights.size());
					_spLights.push_back(*spLight);
				}
					
				Graphics::SubmitLightingData(_pLights, _spLights, *aLight, *dLight);
			}
		}



	}
}

void Assignment::FixedTick()
{

}
