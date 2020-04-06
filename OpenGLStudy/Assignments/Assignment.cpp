#include "Assignment.h"
#include "Application/Window/Window.h"
#include "Application/Window/WindowInput.h"
#include "Constants/Constants.h"



#include "Graphics/Graphics.h"
#include "Time/Time.h"

#include "Graphics/Camera/EditorCamera/EditorCamera.h"
#include "Graphics/Graphics.h"
#include "Material/Material.h"
#include "Actor/Actor.h"
#include "Transform/Transform.h"
#include "Engine/Graphics/Model/Model.h"
#include "Material/Blinn/MatBlinn.h"
#include "Material/PBR_MR/MatPBRMR.h"
#include "Material/Cubemap/MatCubemap.h"
#include "Graphics/Texture/Texture.h"
#include "Assets/Handle.h"
#include "Graphics/EnvironmentCaptureManager.h"
#include <map>
Graphics::ERenderMode g_renderMode = Graphics::ERenderMode::ERM_ForwardShading;

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

	//Graphics::cMatCubemap* _cubemapMat = dynamic_cast<Graphics::cMatCubemap*>(Graphics::cModel::s_manager.Get(m_cubemap->GetModelHandle())->GetMaterialAt());

	//Graphics::cMatBlinn* _teapot2Mat = dynamic_cast<Graphics::cMatBlinn*>(Graphics::cModel::s_manager.Get(m_teapot2->GetModelHandle())->GetMaterialAt());
	//_teapot2Mat->UpdateCubemapTexture(_cubemapMat->GetCubemapHandle());

	//Graphics::cMatPBRMR* _spaceHolderMat = dynamic_cast<Graphics::cMatPBRMR*>(Graphics::cModel::s_manager.Get(m_spaceHolder->GetModelHandle())->GetMaterialAt());


	printf("---------------------------------Game initialization done.---------------------------------\n");
	return result;
}

void Assignment::CreateActor()
{
	m_teapot = new cActor();
	m_teapot->Initialize();
	m_teapot->Transform.SetTransform(glm::vec3(0, 0, 100), glm::quat(glm::vec3(-glm::radians(90.f), 0, 0)), glm::vec3(5, 5, 5));
	m_teapot->SetModel("Contents/models/pbrTeapot.model");

	m_teapot2 = new cActor();
	m_teapot2->Initialize();
	m_teapot2->Transform.SetTransform(glm::vec3(150, 0, 100), glm::quat(glm::vec3(-glm::radians(90.f), 0, 0)), glm::vec3(3, 3, 3));
	m_teapot2->SetModel("Contents/models/pbrTeapot.model");

	m_cubemap = new cActor();
	m_cubemap->Initialize();
	m_cubemap->SetModel("Contents/models/cubemap.model");

	Graphics::cModel* _cubeMap = Graphics::cModel::s_manager.Get(m_cubemap->GetModelHandle());
	Graphics::cMatCubemap* _matCubemap = dynamic_cast<Graphics::cMatCubemap*>(_cubeMap->GetMaterialAt());
	if (_matCubemap)
		_matCubemap->UpdateCubemap(Graphics::GetHDRtoCubemap()->GetCubemapTextureHandle()); // used for debugging the environment

	m_spaceHolder = new cActor();
	m_spaceHolder->Initialize();
	m_spaceHolder->Transform.SetTransform(glm::vec3(0, 150.f, 0), glm::quat(1, 0, 0, 0), glm::vec3(5, 5, 5));
	m_spaceHolder->SetModel("Contents/models/spaceHolder.model");


	for (size_t i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; ++j)
		{
			m_particles[i * 5 + j] = glm::vec3(-100.f + i * 50, 25.f + j * 50, -200);
		}
	}
}

void Assignment::CreateCamera()
{
	m_editorCamera = new  cEditorCamera(glm::vec3(0, 250, 150), 20, 0, 300, 10.f);
	float _aspect = (float)(GetCurrentWindow()->GetBufferWidth()) / (float)(GetCurrentWindow()->GetBufferHeight());
	m_editorCamera->CreateProjectionMatrix(glm::radians(60.f), _aspect, 10.f, 2000.0f);
}

void Assignment::CreateLight()
{
	Graphics::CreateAmbientLight(Color(0.1f, 0.1f, 0.1f), aLight);
	Graphics::CreatePointLight(glm::vec3(-100, 150.f, 100.f), Color(1, 1, 1), 300.f, true, pLight1);
	//Graphics::CreatePointLight(glm::vec3(100, 150.f, 100.f), Color(1, 1, 1), 1.f, 0.7f, 1.8f, true, pLight2);
	Graphics::CreateDirectionalLight(Color(0.6f, 0.6f, 0.6f), glm::vec3(-1, -0.5f, -0.5f), true, dLight);
	//Graphics::CreateSpotLight(glm::vec3(0, 150, 0), glm::vec3(0, 1, 1), Color(1), 65.f, 1.5f, 0.3f, 5.f, true, spLight);
	//Graphics::CreateSpotLight(glm::vec3(100, 150, 0), glm::vec3(1, 1, 0), Color(1), 65.f, 1.f, 0.7f, 

}

void Assignment::SubmitDataToBeRender(const float i_seconds_elapsedSinceLastLoop)
{
	std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>> _renderingMap = std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>>();
	// Submit render setting
	Graphics::SubmitGraphicSettings(g_renderMode);
	// Submit lighting data
	SubmitLightingData();
	// Submit geometry data for shadow map
	SubmitShadowData();
	// Submit post processing data
	Graphics::SubmitPostProcessingData(m_exposureOffset);

	// Frame data from camera
	Graphics::UniformBufferFormats::sFrame _frameData_Camera(m_editorCamera->GetProjectionMatrix(), m_editorCamera->GetViewMatrix());
	// Submit geometry data
	SubmitSceneData(&_frameData_Camera);

	Graphics::SubmitParticleData(m_particles);
	// Gizmos
	{
		// Transform Gizmo
		if (true)
		{
			_renderingMap.clear();
			_renderingMap.reserve(8);
			Assets::cHandle<Graphics::cModel> unneccessaryHandle;
			//cTransform _worldTransform;
			//_renderingMap.push_back({ unneccessaryHandle, _worldTransform });
			//_renderingMap.push_back({ unneccessaryHandle, *m_teapot->Transform() });

			if (pLight1)
				_renderingMap.push_back({ unneccessaryHandle, pLight1->Transform });
/*
			if (pLight2)
				_renderingMap.push_back({ unneccessaryHandle, pLight2->Transform });
			if (spLight)
				_renderingMap.push_back({ unneccessaryHandle, spLight->Transform });
			if (spLight2)
				_renderingMap.push_back({ unneccessaryHandle, spLight2->Transform });*/
			Graphics::SubmitDataToBeRendered(_frameData_Camera, _renderingMap, &Graphics::Gizmo_RenderTransform);
		}
		// Normal Gizmo
		if (false)
		{
			_renderingMap.clear();
			_renderingMap.reserve(1);
			_renderingMap.push_back({ m_teapot2->GetModelHandle(), m_teapot2->Transform });
			Graphics::SubmitDataToBeRendered(_frameData_Camera, _renderingMap, &Graphics::Gizmo_RenderVertexNormal);
		}
		// Triangulation Gizmo
		if (false)
		{
			_renderingMap.clear();
			_renderingMap.reserve(1);
			_renderingMap.push_back({ m_teapot->GetModelHandle(), m_teapot->Transform });
			Graphics::SubmitDataToBeRendered(_frameData_Camera, _renderingMap, &Graphics::Gizmo_RenderTriangulation);
		}
	}
}

void Assignment::BeforeUpdate()
{
	Graphics::MakeApplicationThreadWaitUntilPreRenderFrameDone(m_applicationMutex);
}

void Assignment::Run()
{
	// Clear application thread data
	Graphics::ClearApplicationThreadData();
	// Submit lighting data
	SubmitLightingData();
	// submit shadow data
	SubmitShadowData();
	// submit render requests
	for (int i = 0; i < 6; ++i)
	{
		Graphics::UniformBufferFormats::sFrame _frame;
		SubmitSceneDataForEnvironmentCapture(&_frame);
	}

	Graphics::PreRenderFrame();

	// loop until window closed
	while (!m_shouldApplicationLoopExit)
	{
		// Poll input events
		glfwPollEvents();

		// Render frame
		Graphics::RenderFrame();

		// Swap buffers
		m_window->SwapBuffers();
	}
}

void Assignment::Tick(float second_since_lastFrame)
{
	sWindowInput* _windowInput = m_window->GetWindowInput();

	// get + handle user input events
	{
		m_editorCamera->CameraControl(_windowInput, second_since_lastFrame);

		m_editorCamera->MouseControl(_windowInput, 0.01667f);
	}

	//dLight->Transform()->Rotate(-cTransform::WorldUp, 0.01677f);
	if (m_teapot) {
		m_teapot->Transform.gRotate(glm::vec3(0, 1.f, 0), second_since_lastFrame);
	}
	// Changing exposure
	{
		const float exposureChangeSpeed = 10.f;
		if (_windowInput->IsKeyDown(GLFW_KEY_EQUAL))
		{
			m_exposureOffset += second_since_lastFrame * exposureChangeSpeed;
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_MINUS))
		{
			m_exposureOffset -= second_since_lastFrame * exposureChangeSpeed;
			m_exposureOffset = m_exposureOffset < 0.0001f ? 0.0001f : m_exposureOffset;
		}
	}
	// Changing render mode
	{
		if (_windowInput->IsKeyDown(GLFW_KEY_1))
		{
			g_renderMode = Graphics::ERM_ForwardShading;
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_2))
		{
			g_renderMode = Graphics::ERM_DeferredShading;
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_3))
		{
			g_renderMode = Graphics::ERM_Deferred_Albede;
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_4))
		{
			g_renderMode = Graphics::ERM_Deferred_Metallic;
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_5))
		{
			g_renderMode = Graphics::ERM_Deferred_Roughness;
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_6))
		{
			g_renderMode = Graphics::ERM_Deferred_Normal;
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_7))
		{
			g_renderMode = Graphics::ERM_Deferred_IOR;
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_8))
		{
			g_renderMode = Graphics::ERM_Deferred_Depth;
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_9))
		{
			g_renderMode = Graphics::ERM_Deferred_WorldPos;
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_0))
		{
			g_renderMode = Graphics::ERM_SSAO;
		}
	}

	cTransform* controledActor = nullptr;
	controledActor = &pLight1->Transform;
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
		rotateControl = &spLight->Transform;
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

	m_teapot->Transform.Update();
	m_teapot2->Transform.Update();

	if (pLight1)
		pLight1->Transform.Update();
	if (pLight2)
	{
		pLight2->Transform.Update();
	}

	if (dLight)
		dLight->Transform.Update();
	if (spLight)
		spLight->Transform.Update();
	if (spLight2)
		spLight2->Transform.Update();

}

void Assignment::SubmitLightingData()
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
	if (spLight2)
	{
		spLight2->UpdateLightIndex(_spLights.size());
		_spLights.push_back(*spLight2);
	}
	Graphics::SubmitLightingData(_pLights, _spLights, *aLight, *dLight);
}

void Assignment::SubmitSceneData(Graphics::UniformBufferFormats::sFrame* const i_frameData)
{
	std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>> _renderingMap;

	// PBR pass
	{
		_renderingMap.clear();
		_renderingMap.reserve(3);
		_renderingMap.push_back({ m_spaceHolder->GetModelHandle(), m_spaceHolder->Transform });
		_renderingMap.push_back({ m_teapot->GetModelHandle(), m_teapot->Transform });
		_renderingMap.push_back({ m_teapot2->GetModelHandle(), m_teapot2->Transform });
		Graphics::SubmitDataToBeRendered(*i_frameData, _renderingMap, &Graphics::PBR_Pass);
	}

	// Cube map
	{
		_renderingMap.clear();
		_renderingMap.push_back({ m_cubemap->GetModelHandle(), m_cubemap->Transform });
		Graphics::UniformBufferFormats::sFrame _frameData_Cubemap(m_editorCamera->GetProjectionMatrix(), glm::mat4(glm::mat3(m_editorCamera->GetViewMatrix())));
		Graphics::SubmitDataToBeRendered(*i_frameData, _renderingMap, &Graphics::CubeMap_Pass);
	}
}

void Assignment::SubmitSceneDataForEnvironmentCapture(Graphics::UniformBufferFormats::sFrame* const i_frameData)
{
	std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>> _renderingMap;
	// PBR pass
	{
		_renderingMap.clear();
		_renderingMap.reserve(3);
		_renderingMap.push_back({ m_spaceHolder->GetModelHandle(), m_spaceHolder->Transform });
		_renderingMap.push_back({ m_teapot->GetModelHandle(), m_teapot->Transform });
		_renderingMap.push_back({ m_teapot2->GetModelHandle(), m_teapot2->Transform });
		Graphics::SubmitDataToBeRendered(*i_frameData, _renderingMap, &Graphics::PBR_Pass);
	}

	// Cube map
	{
		_renderingMap.clear();
		_renderingMap.push_back({ m_cubemap->GetModelHandle(),m_cubemap->Transform });
		Graphics::UniformBufferFormats::sFrame _frameData_Cubemap(m_editorCamera->GetProjectionMatrix(), glm::mat4(glm::mat3(m_editorCamera->GetViewMatrix())));
		Graphics::SubmitDataToBeRendered(*i_frameData, _renderingMap, &Graphics::CubeMap_Pass);
	}
}

void Assignment::SubmitShadowData()
{
	std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>> _renderingMap;

	_renderingMap.reserve(32);
	_renderingMap.push_back({ m_teapot->GetModelHandle(), m_teapot->Transform });
	_renderingMap.push_back({ m_teapot2->GetModelHandle(), m_teapot2->Transform });
	_renderingMap.push_back({ m_spaceHolder->GetModelHandle(), m_spaceHolder->Transform });

	{// Spot light shadow map pass
		Graphics::SubmitDataToBeRendered(Graphics::UniformBufferFormats::sFrame(), _renderingMap, &Graphics::SpotLightShadowMap_Pass);
	}

	{ // Point light shadow map pass
		Graphics::SubmitDataToBeRendered(Graphics::UniformBufferFormats::sFrame(), _renderingMap, &Graphics::PointLightShadowMap_Pass);
	}

	// Frame data from directional light
	if (dLight && dLight->IsShadowEnabled()) {
		Graphics::UniformBufferFormats::sFrame _frameData_Shadow(dLight->GetProjectionmatrix(), dLight->GetViewMatrix());
		// directional light shadow map pass
		Graphics::SubmitDataToBeRendered(_frameData_Shadow, _renderingMap, &Graphics::DirectionalShadowMap_Pass);
	}

}

void Assignment::FixedTick()
{

}

void Assignment::CleanUp()
{
	safe_delete(m_editorCamera);
	safe_delete(m_teapot);
	safe_delete(m_teapot2);
	safe_delete(m_cubemap);
	safe_delete(m_spaceHolder);
}
