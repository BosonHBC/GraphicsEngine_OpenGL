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

#include "Assignments/ClothSimulation/SimulationParams.h"
#include <random>
#include "Application/imgui/imgui.h"


Graphics::ERenderMode g_renderMode = Graphics::ERenderMode::ERM_ForwardShading;

std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
std::default_random_engine generator;

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

	m_renderingTeapotCount = glm::clamp(m_renderingTeapotCount, 1, s_maxTeapotCount);
	m_createdPLightCount = glm::clamp(m_createdPLightCount, 1, s_maxPLightCount);

	aLight->SetColor(Color(0.1f) * m_ambientIntensity);
	dLight->SetColor(Color(0.6f, 0.6f, 0.5f)* m_directionalIntensity);

	printf("---------------------------------Game initialization done.---------------------------------\n");
	return result;
}

const char* g_teapotPaths[Assignment::s_maxTeapotCount] =
{
	"Contents/models/pbrTeapot.model",
	"Contents/models/pbrTeapot_rustedIron.model",
	"Contents/models/pbrTeapot_caveFloor.model",
	"Contents/models/pbrTeapot_wornPainted.model",
	"Contents/models/pbrTeapot_ChipedPaintMetal.model",
	"Contents/models/pbrTeapot_RustPanel.model",
	"Contents/models/pbrTeapot_SpeckledCountertop.model",
	"Contents/models/pbrTeapot_WinkledPaper.model"
};
void Assignment::CreateActor()
{
	const int teapotPerRow = 4;
	const float horiDist = 100;
	const float vertDist = 100;
	for (int i = 0; i < m_renderingTeapotCount; ++i)
	{
		m_teapots[i] = new cActor();
		m_teapots[i]->Initialize();
		m_teapots[i]->Transform.SetTransform(glm::vec3(-150 + (i % teapotPerRow) * horiDist, 0, 100 - (i / teapotPerRow) * vertDist), glm::quat(glm::vec3(-glm::radians(90.f), glm::radians(30.f), 0)), glm::vec3(5, 5, 5));
		m_teapots[i]->SetModel(g_teapotPaths[i]);
	}


	m_cubemap = new cActor();
	m_cubemap->Initialize();
	m_cubemap->SetModel("Contents/models/cubemap.model");

	Graphics::cModel* _cubeMap = Graphics::cModel::s_manager.Get(m_cubemap->GetModelHandle());
	Graphics::cMatCubemap* _matCubemap = dynamic_cast<Graphics::cMatCubemap*>(_cubeMap->GetMaterialAt());
	if (_matCubemap)
		_matCubemap->UpdateCubemap(Graphics::GetHDRtoCubemap()->GetCubemapTextureHandle());

	m_spaceHolder = new cActor();
	m_spaceHolder->Initialize();
	m_spaceHolder->Transform.SetTransform(glm::vec3(0, 150.f, 0), glm::quat(1, 0, 0, 0), glm::vec3(5, 5, 5));
	m_spaceHolder->SetModel("Contents/models/spaceHolder.model");

	m_collisionSphere = new cActor();
	m_collisionSphere->Initialize();
	m_collisionSphere->Transform.SetTransform(glm::vec3(0, 0, -150), glm::quat(1, 0, 0, 0), glm::vec3(10, 10, 10));
	m_collisionSphere->SetModel("Contents/models/pbrSphere.model");
}

void Assignment::CreateCamera()
{
	//m_editorCamera = new  cEditorCamera(glm::vec3(0, 250, 150), 20, 0, 300, 10.f);
	m_editorCamera = new  cEditorCamera(glm::vec3(0, 200, 350), 25, 0, 300, 10.f);
	float _aspect = (float)(GetCurrentWindow()->GetBufferWidth()) / (float)(GetCurrentWindow()->GetBufferHeight());
	m_editorCamera->CreateProjectionMatrix(glm::radians(60.f), _aspect, 10.f, 2000.0f);
}

void Assignment::CreateLight()
{
	const int lightPerRow = 4;
	const float horiDist = 150;
	const float vertDist = 50 * 40.f / m_createdPLightCount;
	Graphics::CreateAmbientLight(Color(0, 0, 0), aLight);

	for (int i = 0; i < m_createdPLightCount; ++i)
	{
		//Color randomColor = Color(randomFloats(generator), randomFloats(generator), randomFloats(generator));
		bool enableShadow = true;
		Graphics::CreatePointLight(glm::vec3(0 + (i % lightPerRow) * horiDist, 200, 100 - (i / lightPerRow) * vertDist), Color::White() * 0.5f, 250.f, enableShadow, m_pLights[i]);
	}

	//CreatePointLight(glm::vec3(0, 100, 0), Color::White() * 0.5f, 250, true);

	//Graphics::CreatePointLight(glm::vec3(100, 150.f, 100.f), Color(1, 1, 1), 1.f, 0.7f, 1.8f, true, pLight2);
	Graphics::CreateDirectionalLight(Color(0.00f, 0.00f, 0.00f), glm::vec3(-1, -0.5f, -0.5f), true, dLight);
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

	if (ClothSim::g_bEnableClothSim)
		Graphics::SubmitParticleData();
	// Gizmos
	{
		// Transform Gizmo
		if (false)
		{
			_renderingMap.clear();
			_renderingMap.reserve(8);
			Assets::cHandle<Graphics::cModel> unneccessaryHandle;
			//cTransform _worldTransform;
			//_renderingMap.push_back({ unneccessaryHandle, _worldTransform });
			//_renderingMap.push_back({ unneccessaryHandle, *m_teapot->Transform() });

			Graphics::SubmitDataToBeRendered(_frameData_Camera, _renderingMap, &Graphics::Gizmo_RenderTransform);
		}
		// Normal Gizmo
/*
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
		}*/
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
		RenderEditorGUI();

		// Swap buffers
		m_window->SwapBuffers();
	}
}


void Assignment::Tick(float second_since_lastFrame)
{
	sWindowInput* _windowInput = GetCurrentWindow()->GetWindowInput();
	if (ImGui::IsKeyDown(GLFW_KEY_ESCAPE)) {
		// tell glfw window that it is time to close
		glfwSetWindowShouldClose(GetCurrentWindow()->GetWindow(), GL_TRUE);
	}
	// get + handle user input events
	{
		m_editorCamera->CameraControl(_windowInput, second_since_lastFrame);

		m_editorCamera->MouseControl(_windowInput, 0.01667f);
	}
	cTransform* controledActor = nullptr;
	controledActor = &m_pLights[0]->Transform;
	//controledActor = m_sphere->Transform();
	if (controledActor) {
		if (ImGui::IsKeyDown(GLFW_KEY_J)) {
			for (int i = 0; i < m_createdPLightCount; ++i)
			{
				m_pLights[i]->Transform.Translate(-cTransform::WorldRight * 100.f * second_since_lastFrame);
			}
		}
		if (ImGui::IsKeyDown(GLFW_KEY_L)) {
			for (int i = 0; i < m_createdPLightCount; ++i)
			{
				m_pLights[i]->Transform.Translate(cTransform::WorldRight* 100.f  * second_since_lastFrame);
			}
		}
		if (ImGui::IsKeyDown(GLFW_KEY_I)) {
			for (int i = 0; i < m_createdPLightCount; ++i)
			{
				m_pLights[i]->Transform.Translate(-cTransform::WorldForward* 100.f  * second_since_lastFrame);
			}
		}
		if (ImGui::IsKeyDown(GLFW_KEY_K)) {
			for (int i = 0; i < m_createdPLightCount; ++i)
			{
				m_pLights[i]->Transform.Translate(cTransform::WorldForward* 100.f  * second_since_lastFrame);
			}
		}
		if (ImGui::IsKeyDown(GLFW_KEY_SPACE)) {
			for (int i = 0; i < m_createdPLightCount; ++i)
			{
				m_pLights[i]->Transform.Translate(cTransform::WorldUp* 100.f* second_since_lastFrame);
			}
		}
		if (ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
			for (int i = 0; i < m_createdPLightCount; ++i)
			{
				m_pLights[i]->Transform.Translate(-cTransform::WorldUp* 100.f * second_since_lastFrame);
			}
		}

	}
	// ClothSim
	{
		float nodeMoveSpeed = 50.f;
		if (ImGui::IsKeyDown(GLFW_KEY_LEFT)) {
			ClothSim::MoveFixedNode(glm::vec3(-1, 0, 0) *nodeMoveSpeed * second_since_lastFrame);
		}
		if (ImGui::IsKeyDown(GLFW_KEY_RIGHT)) {
			ClothSim::MoveFixedNode(glm::vec3(1, 0, 0) *nodeMoveSpeed * second_since_lastFrame);
		}
		if (ImGui::IsKeyDown(GLFW_KEY_UP)) {
			ClothSim::MoveFixedNode(glm::vec3(0, 0, -1) *nodeMoveSpeed * second_since_lastFrame);
		}
		if (ImGui::IsKeyDown(GLFW_KEY_DOWN)) {
			ClothSim::MoveFixedNode(glm::vec3(0, 0, 1) *nodeMoveSpeed * second_since_lastFrame);
		}
		if (ImGui::IsKeyDown(GLFW_KEY_Y)) {
			ClothSim::MoveFixedNode(glm::vec3(0, 1, 0) *nodeMoveSpeed * second_since_lastFrame);
		}
		if (ImGui::IsKeyDown(GLFW_KEY_H)) {
			ClothSim::MoveFixedNode(glm::vec3(0, -1, 0) *nodeMoveSpeed * second_since_lastFrame);
		}
		if (ImGui::IsKeyDown(GLFW_KEY_Z)) {
			ClothSim::ScaleFixedNode(glm::vec3(-1, 0, 0) *nodeMoveSpeed * second_since_lastFrame);
		}
		if (ImGui::IsKeyDown(GLFW_KEY_X)) {
			ClothSim::ScaleFixedNode(glm::vec3(1, 0, 0) *nodeMoveSpeed * second_since_lastFrame);
		}
	}

	if (ImGui::IsKeyPressed(GLFW_KEY_G))
	{
		CreatePointLight(m_editorCamera->CamLocation() + m_editorCamera->Transform.Forward() * 100.f, Color::White() * 0.5f, 250.f, true);
		printf("Current Point Light Count: %d\n", m_createdPLightCount);
	}

	for (int i = 0; i < m_renderingTeapotCount; ++i)
	{
		m_teapots[i]->Transform.Update();
	}

	for (int i = 0; i < m_createdPLightCount; ++i)
	{
		m_pLights[i]->Transform.Update();
	}

	if (dLight)
		dLight->Transform.Update();
	if (spLight)
		spLight->Transform.Update();
	if (spLight2)
		spLight2->Transform.Update();


	if (ClothSim::g_bEnableClothSim)
	{
		for (int i = 0; i < m_createdPLightCount; ++i)
		{
			m_collisionSpheres[i].SetCenter(m_pLights[i]->Transform.Position());
			m_collisionSpheres[i].SetRadius(m_pLights[i]->Transform.Scale().x / 8.f);
		}
		ClothSim::UpdateSprings(0.05f, m_collisionSpheres, m_createdPLightCount);
	}
}

void Assignment::SubmitLightingData()
{

	std::vector<Graphics::cPointLight> _pLights;
	std::vector<Graphics::cSpotLight> _spLights;

	for (int i = 0; i < m_createdPLightCount; ++i)
	{
		m_pLights[i]->UpdateLightIndex(_pLights.size());
		m_pLights[i]->CalculateDistToEye(m_editorCamera->CamLocation());
		_pLights.push_back(*m_pLights[i]);
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
		_renderingMap.reserve(m_renderingTeapotCount + 2);
		_renderingMap.push_back({ m_spaceHolder->GetModelHandle(), m_spaceHolder->Transform });
		for (int i = 0; i < m_renderingTeapotCount; ++i)
		{
			_renderingMap.push_back({ m_teapots[i]->GetModelHandle(), m_teapots[i]->Transform });
		}

		if (ClothSim::g_bEnableClothSim)
			_renderingMap.push_back({ m_collisionSphere->GetModelHandle(), m_collisionSphere->Transform });

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
		_renderingMap.reserve(m_renderingTeapotCount + 2);
		_renderingMap.push_back({ m_spaceHolder->GetModelHandle(), m_spaceHolder->Transform });
		for (int i = 0; i < m_renderingTeapotCount; ++i)
		{
			_renderingMap.push_back({ m_teapots[i]->GetModelHandle(), m_teapots[i]->Transform });
		}
		if (ClothSim::g_bEnableClothSim)
			_renderingMap.push_back({ m_collisionSphere->GetModelHandle(), m_collisionSphere->Transform });

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

	_renderingMap.reserve(m_renderingTeapotCount + 2);
	for (int i = 0; i < m_renderingTeapotCount; ++i)
	{
		_renderingMap.push_back({ m_teapots[i]->GetModelHandle(), m_teapots[i]->Transform });
	}
	_renderingMap.push_back({ m_spaceHolder->GetModelHandle(), m_spaceHolder->Transform });

	if (ClothSim::g_bEnableClothSim)
		_renderingMap.push_back({ m_collisionSphere->GetModelHandle(), m_collisionSphere->Transform });

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

void Assignment::CreatePointLight(const glm::vec3& i_initialLocation, const Color& i_color, const GLfloat& i_radius, bool i_enableShadow)
{
	if (m_createdPLightCount < s_maxPLightCount)
		Graphics::CreatePointLight(i_initialLocation, i_color, i_radius, i_enableShadow, m_pLights[m_createdPLightCount++]);

}

void Assignment::FixedTick()
{


}

void Assignment::EditorGUI()
{
	ImGui::Begin("Status");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("Point light count: %d", m_createdPLightCount);
	ImGui::End();

	ImGui::Begin("Control");
	const Graphics::ERenderMode rendermodes[] = {Graphics::ERM_ForwardShading, Graphics::ERM_DeferredShading, Graphics::ERM_Deferred_Albede, Graphics::ERM_Deferred_Metallic, Graphics::ERM_Deferred_Roughness, 
		Graphics::ERM_Deferred_Normal, Graphics::ERM_Deferred_Depth, Graphics::ERM_SSAO };
	static bool toggles[] = { false, true, false, false, false, false, false, false };

	if (ImGui::Button("ChooseRenderMode.."))
		ImGui::OpenPopup("shading_mode_popup");
	ImGui::SameLine();
	ImGui::TextUnformatted(Graphics::g_renderModeNameMap.at(static_cast<uint8_t>(g_renderMode)));
	if (ImGui::BeginPopup("shading_mode_popup"))
	{
		ImGui::Text("Shading Modes:");
		ImGui::Separator();
		for (int i = 0; i < IM_ARRAYSIZE(rendermodes); i++)
			if (ImGui::Selectable(Graphics::g_renderModeNameMap.at(static_cast<uint8_t>(rendermodes[i]))))
			{
				g_renderMode = rendermodes[i];
			}
		ImGui::EndPopup();
	}


	if(ImGui::DragFloat("Ambient", &m_ambientIntensity,0.1f, 0.0f, 10.0f))
		aLight->SetColor(Color(0.1f, 0.1f, 0.1f) * m_ambientIntensity);
	if(ImGui::DragFloat("Directional", &m_directionalIntensity, 0.1f, 0.0f, 3.0f))
		dLight->SetColor(Color(0.6f, 0.6f, 0.5f) * m_directionalIntensity);
	ImGui::DragFloat("Exposure", &m_exposureOffset, 0.1f, 0.0001f, 50.0f);
	if (ImGui::Button("Reset"))
	{
		g_renderMode = Graphics::ERM_DeferredShading;
		m_ambientIntensity = 1.0f;
		m_directionalIntensity = 1.0f;
		m_exposureOffset = 3.0f;
	}
	ImGui::End();
}

void Assignment::CleanUp()
{
	safe_delete(m_collisionSphere);
	safe_delete(m_editorCamera);
	for (int i = 0; i < m_renderingTeapotCount; ++i)
	{
		safe_delete(m_teapots[i]);
	}
	safe_delete(m_cubemap);
	safe_delete(m_spaceHolder);
	ClothSim::CleanUpData();
}
