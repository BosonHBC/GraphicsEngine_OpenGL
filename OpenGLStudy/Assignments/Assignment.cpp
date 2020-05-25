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
#include "Editor/Editor.h"
#include "Cores/Utility//Profiler.h"
#include "Assignments/ParticleTest.h"

Graphics::ERenderMode g_renderMode = Graphics::ERenderMode::ERM_DeferredShading;

std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
std::default_random_engine generator;

std::vector<cActor*> g_renderActorList;

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

	m_renderingTeapotCount = glm::clamp(m_renderingTeapotCount, 0, s_maxTeapotCount);
	m_createdPLightCount = glm::clamp(m_createdPLightCount, 0, s_maxPLightCount);

	m_ppData.Exposure = 0.9f;
	m_ppData.EnableFxAA = true;
	m_ppData.EnablePostProcessing = true;
	m_ppData.ScreenResolution = glm::vec2(static_cast<float>(GetCurrentWindow()->GetBufferWidth()), static_cast<float>(GetCurrentWindow()->GetBufferHeight()));
	m_ppData.TonemappingMode = 1;

	m_ssaoRadius = 20;
	m_ssaoPower = 1.2f;
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
		g_renderActorList.push_back(m_teapots[i]);
	}

	m_cubemap = new cActor();
	m_cubemap->Initialize();
	m_cubemap->SetModel("Contents/models/cubemap.model");

	Graphics::cMatCubemap* _matCubemap = dynamic_cast<Graphics::cMatCubemap*>(Graphics::cMaterial::s_manager.Get(m_cubemap->GetModelHandle().GetMaterialAt()));
	if (_matCubemap)
		_matCubemap->UpdateCubemap(Graphics::GetHDRtoCubemap()->GetCubemapTextureHandle());

	m_spaceHolder = new cActor();
	m_spaceHolder->Initialize();
	m_spaceHolder->Transform.SetTransform(glm::vec3(0, 150.f, 0), glm::quat(1, 0, 0, 0), glm::vec3(5, 5, 5));
	m_spaceHolder->SetModel("Contents/models/spaceHolder.model");
	g_renderActorList.push_back(m_spaceHolder);

	m_supplyBox = new cActor();
	m_supplyBox->Initialize();
	m_supplyBox->Transform.SetTransform(glm::vec3(0, 0, -10), glm::quat(1, 0, 0, 0), glm::vec3(250, 250, 250));
	m_supplyBox->SetModel("Contents/models/sifiGuns/supplyBox_Close.model");
	g_renderActorList.push_back(m_supplyBox);
#ifdef ENABLE_CLOTH_SIM
	m_collisionSphere = new cActor();
	m_collisionSphere->Initialize();
	m_collisionSphere->Transform.SetTransform(glm::vec3(0, 0, -150), glm::quat(1, 0, 0, 0), glm::vec3(10, 10, 10));
	m_collisionSphere->SetModel("Contents/models/pbrSphere.model");
	g_renderActorList.push_back(m_collisionSphere);
#endif // ENABLE_CLOTH_SIM

}

void Assignment::CreateCamera()
{
	//m_editorCamera = new  cEditorCamera(glm::vec3(0, 250, 150), 20, 0, 300, 10.f);
	m_editorCamera = new  cEditorCamera(glm::vec3(0, 200, 350), 25, 0, 300, 15.f);
	float _aspect = (float)(GetCurrentWindow()->GetBufferWidth()) / (float)(GetCurrentWindow()->GetBufferHeight());
	m_editorCamera->CreateProjectionMatrix(glm::radians(60.f), _aspect, 10.f, 2000.0f);
}

void Assignment::CreateLight()
{
	const int lightPerRow = 4;
	const float horiDist = 50 * 40.f / m_createdPLightCount;
	const float vertDist = 50 * 40.f / m_createdPLightCount;
	Graphics::CreateAmbientLight(Color::White(), aLight);

	for (int i = 0; i < m_createdPLightCount; ++i)
	{
		//Color randomColor = Color(randomFloats(generator), randomFloats(generator), randomFloats(generator));
		bool enableShadow = true;
		Graphics::CreatePointLight(glm::vec3(0 + (i % lightPerRow) * horiDist, 200, 100 - (i / lightPerRow) * vertDist), m_pointLightColor, 250.f, enableShadow, m_pLights[i]);
	}

	//CreatePointLight(glm::vec3(0, 100, 0), Color::White() * 0.5f, 250, true);

	//Graphics::CreatePointLight(glm::vec3(100, 150.f, 100.f), Color(1, 1, 1), 1.f, 0.7f, 1.8f, true, pLight2);
	Graphics::CreateDirectionalLight(Color(0.6f, 0.6f, 0.5f), glm::vec3(-1, -0.5f, -0.5f), true, dLight);
	//Graphics::CreateSpotLight(glm::vec3(0, 150, 0), glm::vec3(0, 1, 1), Color(1), 65.f, 1.5f, 0.3f, 5.f, true, spLight);
	//Graphics::CreateSpotLight(glm::vec3(100, 150, 0), glm::vec3(1, 1, 0), Color(1), 65.f, 1.f, 0.7f, 

}

void Assignment::SubmitDataToBeRender(const float i_seconds_elapsedSinceLastLoop)
{
	// Submit render setting
	Graphics::SubmitGraphicSettings(g_renderMode);

	Profiler::StartRecording(0);
	// Submit lighting data
	SubmitLightingData();
	Profiler::StopRecording(0);

	// Submit geometry data for shadow map
	SubmitShadowData();


	// Submit post processing data
	Graphics::SubmitPostProcessingData(m_ppData, m_ssaoRadius, m_ssaoPower);

	// Submit IO Data
	glm::vec2 mousePos = glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	const auto& dragDelta = ImGui::GetMouseDragDelta();
	glm::vec2 mouseDelta(dragDelta.x, dragDelta.y);
	bool downs[3] = { false };
	for (int i = 0; i < 3; ++i) downs[i] = ImGui::IsMouseDown(i);
	Graphics::SubmitIOData(mousePos, mouseDelta, downs);


	// Frame data from camera
	Graphics::UniformBufferFormats::sFrame _frameData_Camera(m_editorCamera->GetProjectionMatrix(), m_editorCamera->GetViewMatrix());
	std::vector<std::pair<Graphics::cModel, cTransform>> _renderingMap;
	_renderingMap.reserve(g_renderActorList.size());

	// Submit geometry data
	SubmitSceneData(_renderingMap, &_frameData_Camera);

	// Submit Selection data
	Graphics::SubmitSelectionData(Editor::SelectingItemID, _renderingMap);

#ifdef ENABLE_CLOTH_SIM
	Graphics::SubmitParticleData();
#endif // ENABLE_CLOTH_SIM
	// Gizmos
	{
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
		Profiler::StartRecording(1);
		Graphics::RenderFrame();
		Profiler::StopRecording(1);

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

		m_editorCamera->MouseControl(_windowInput, second_since_lastFrame);
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
#ifdef ENABLE_CLOTH_SIM
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
#endif // ENABLE_CLOTH_SIM

	auto& dataFromRenderThread = Graphics::GetDataFromRenderThread();

	for (int i = 0; i < g_renderActorList.size(); ++i)
	{
		g_renderActorList[i]->Transform.Update();
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

#ifdef ENABLE_CLOTH_SIM
	for (int i = 0; i < m_createdPLightCount; ++i)
	{
		m_collisionSpheres[i].SetCenter(m_pLights[i]->Transform.Position());
		m_collisionSpheres[i].SetRadius(m_pLights[i]->Transform.Scale().x / 8.f);
	}
	ClothSim::UpdateSprings(0.05f, m_collisionSpheres, m_createdPLightCount);
#endif // ENABLE_CLOTH_SIM

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
	// process point light
	{
		std::sort(_pLights.begin(), _pLights.end(), [](Graphics::cPointLight& const l1, Graphics::cPointLight&  const l2) {
			return l1.Importance() > l2.Importance(); });
		// Now the point light list is sorted depends on their importance
		for (size_t i = 0; i < _pLights.size(); ++i)
		{
			auto* it = &_pLights[i];
			int shadowMapIdx = -1; int resolutionIdx = -1;
			if (Graphics::RetriveShadowMapIndexAndSubRect(i, shadowMapIdx, resolutionIdx))
			{
				it->SetShadowmapIdxAndResolutionIdx(shadowMapIdx, resolutionIdx);
				it->ImportanceOrder = i;
			}
			else
				assert(false);
		}
		std::sort(_pLights.begin(), _pLights.end(), [](Graphics::cPointLight& const l1, Graphics::cPointLight&  const l2) {
			return l1.ShadowMapIdx() < l2.ShadowMapIdx(); });
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

void Assignment::SubmitSceneData(std::vector<std::pair<Graphics::cModel, cTransform>>& io_sceneData, Graphics::UniformBufferFormats::sFrame* const i_frameData)
{
	// PBR pass
	{
		// io_sceneData can be reuse in selection pass
		io_sceneData.clear();
		for (int i = 0; i < g_renderActorList.size(); ++i)
		{
			io_sceneData.push_back({ g_renderActorList[i]->GetModelHandle(), g_renderActorList[i]->Transform });
		}
		Graphics::SubmitDataToBeRendered(*i_frameData, io_sceneData, &Graphics::PBR_Pass);
}

	// Cube map
	{
		std::vector<std::pair<Graphics::cModel, cTransform>> _renderingMap;
		_renderingMap.push_back({ m_cubemap->GetModelHandle(), m_cubemap->Transform });
		Graphics::UniformBufferFormats::sFrame _frameData_Cubemap(m_editorCamera->GetProjectionMatrix(), glm::mat4(glm::mat3(m_editorCamera->GetViewMatrix())));
		Graphics::SubmitDataToBeRendered(*i_frameData, _renderingMap, &Graphics::CubeMap_Pass);
	}
}

void Assignment::SubmitSceneDataForEnvironmentCapture(Graphics::UniformBufferFormats::sFrame* const i_frameData)
{
	std::vector<std::pair<Graphics::cModel, cTransform>> _renderingMap;
	// PBR pass
	{
		_renderingMap.clear();
		_renderingMap.reserve(g_renderActorList.size());
		for (int i = 0; i < g_renderActorList.size(); ++i)
		{
			_renderingMap.push_back({ g_renderActorList[i]->GetModelHandle(), g_renderActorList[i]->Transform });
		}

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
	std::vector<std::pair<Graphics::cModel, cTransform>> _renderingMap;

	_renderingMap.reserve(g_renderActorList.size());
	for (int i = 0; i < g_renderActorList.size(); ++i)
	{
		_renderingMap.push_back({ g_renderActorList[i]->GetModelHandle(), g_renderActorList[i]->Transform });
	}

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
	std::lock_guard<std::mutex> autoLock(m_applicationMutex);
	if (m_createdPLightCount < s_maxPLightCount)
	{
		Graphics::CreatePointLight(i_initialLocation, i_color, i_radius, i_enableShadow, m_pLights[m_createdPLightCount]);
		m_createdPLightCount++;
	}


}

void Assignment::FixedTick()
{


}

void Assignment::EditorGUI()
{
	auto& dataFromRenderThread = Graphics::GetDataFromRenderThread();
	// handle selecting
	{
		int hoverArrowDirection = -1;
		bool isHoveringTransformGizmo = Graphics::IsTransformGizmoIsHovered(dataFromRenderThread.g_selectionID, hoverArrowDirection);
		glm::vec4 dragDelta(glm::vec2(ImGui::GetIO().MouseDelta.x, -ImGui::GetIO().MouseDelta.y), 0, 0);
		static bool dragingTransform = false;
		static int draggingDirection = -1;
		if (!ImGui::GetIO().WantCaptureMouse)
		{
			bool isReleased = ImGui::IsMouseReleased(0);

			// Only when 1. release the mouse; 2. not hovering a transform; 3. not on a UI; 4. not dragging
			if (isReleased && !isHoveringTransformGizmo && !dragingTransform
				&& IsFloatZero(dragDelta.x) && IsFloatZero(dragDelta.y))
			{
				Editor::SelectingItemID = dataFromRenderThread.g_selectionID;
			}

			if (isHoveringTransformGizmo && !dragingTransform && ImGui::IsMouseDown(0))
			{
				dragingTransform = true;
				draggingDirection = hoverArrowDirection;
				Graphics::SetArrowBeingSelected(true, draggingDirection);
			}

			if (dragingTransform && ImGui::IsMouseDown(0) && ISelectable::IsValid(Editor::SelectingItemID) && (!IsFloatZero(dragDelta.x) || !IsFloatZero(dragDelta.y)))
			{
				cTransform* edittingTransform = nullptr;
				if (ISelectable::s_selectableList[Editor::SelectingItemID]->GetBoundTransform(edittingTransform))
				{
					glm::vec3 dragInWorldSpace = m_editorCamera->GetInvViewMatrix() * dragDelta;
					float dragVelocity = glm::length(dragInWorldSpace);
					glm::vec3 dragInWorldSpace_norm = dragInWorldSpace / dragVelocity;
					glm::vec3 direction(0.0f);
					switch (draggingDirection)
					{
					case 0:
						direction = edittingTransform->Forward();
						break;
					case 1:
						direction = edittingTransform->Right();
						break;
					case 2:
						direction = edittingTransform->Up();
						break;
					default:
						break;
					}

					float directionSign = glm::dot(direction, dragInWorldSpace_norm) > 0 ? 1 : -1;
					edittingTransform->Translate(direction * directionSign * 100.f * dragVelocity * 0.02f);
				}
			}

			if (dragingTransform && isReleased)
			{
				dragingTransform = false;
				Graphics::SetArrowBeingSelected(false, draggingDirection);
				draggingDirection = -1;
			}
		}
	}


	ImGui::Begin("Status");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	if (ImGui::TreeNode("Frame rate break downs"))
	{
		ImGui::Columns(2, "break downs");
		ImGui::Separator();
		ImGui::TextWrapped("CPU delta time");

		ImGui::TextWrapped("Submit lighting data");
		ImGui::TextWrapped("Render a frame CPU");
		ImGui::TextWrapped("Deferred shading CPU");
		ImGui::TextWrapped("HDR CPU");
		ImGui::TextWrapped("Render Point light CPU");
		ImGui::TextWrapped("Editor CPU");
		ImGui::TextWrapped("Render thread waiting time CPU");

		ImGui::NextColumn();

		ImGui::TextWrapped("%.4f ms/frame", Time::DeltaTime() * 1000);
		float f;
		for (int i = 0; i < 7; ++i)
		{
			Profiler::GetProfilingTime(i, f);
			ImGui::TextWrapped("%.4f ms/frame", f);
		}

		/*
				ImGui::TextWrapped("%.4f ms/frame", dataFromRenderThread.g_deltaRenderAFrameTime);
				ImGui::TextWrapped("%.4f ms/frame", dataFromRenderThread.g_deltaGeometryTime);
				ImGui::TextWrapped("%.4f ms/frame", dataFromRenderThread.g_deltaDeferredLightingTime);
				ImGui::TextWrapped("%.4f ms/frame", dataFromRenderThread.g_deltaPointLightShadowMapTime);
				ImGui::TextWrapped("%.4f ms/frame", dataFromRenderThread.g_deltaSelectionTime);*/
		ImGui::Columns(1);
		ImGui::Separator();

		ImGui::TreePop();
	}
	ImGui::Text("Point light count: %d", m_createdPLightCount);
	ImGui::Text("Tile intersects by light: %d", dataFromRenderThread.g_tilesIntersectByLight);
	ImGui::End();

	ImGui::Begin("Control");
	{
		if (ImGui::CollapsingHeader("Render settings"))
		{
			const Graphics::ERenderMode rendermodes[] = { Graphics::ERM_ForwardShading, Graphics::ERM_DeferredShading, Graphics::ERM_Deferred_Albede, Graphics::ERM_Deferred_Metallic, Graphics::ERM_Deferred_Roughness,
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
		}
		if (ImGui::CollapsingHeader("Lighting"))
		{
			ImGui::DragFloat("Ambient intensity", &aLight->Intensity, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("Directional light intensity", &dLight->Intensity, 0.5f, 0.0f, 1000.0f);

			ImGui::ColorEdit3("Point Light Color: ", (float*)&m_pointLightColor);
			if (ImGui::Button("Spawn Point Light"))
			{
				CreatePointLight(m_editorCamera->CamLocation() + m_editorCamera->Transform.Forward() * 100.f, m_pointLightColor, 250.f, true);
			}
		}
		if (ImGui::CollapsingHeader("Post processing"))
		{
			static bool _enablePP = m_ppData.EnablePostProcessing;
			ImGui::Checkbox("Enable Post processing", &_enablePP);
			m_ppData.EnablePostProcessing = _enablePP;
			if (_enablePP)
			{
				ImGui::PushItemWidth(100);
				if (ImGui::TreeNode("Tone mapping mode"))
				{
					const const char* modes[4] = { "No Tone mapping", "Reinhard", "Filmic", "Uncharted" };
					static int selected = m_ppData.TonemappingMode + 1;
					for (int n = 0; n < 4; n++)
					{
						if (ImGui::Selectable(modes[n], selected == n))
							selected = n;
					}
					m_ppData.TonemappingMode = selected - 1;
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("SSAO settings"))
				{
					ImGui::DragFloat("SSAO radius", &m_ssaoRadius, 0.01f, 5.f, 30.0f);
					ImGui::DragFloat("SSAO power", &m_ssaoPower, 0.01f, 1.f, 10.0f);

					ImGui::TreePop();
				}
				ImGui::DragFloat("Exposure", &m_ppData.Exposure, 0.01f, 0.0001f, 50.0f);
				ImGui::PopItemWidth();
				static bool _enableFxAA = m_ppData.EnableFxAA;
				ImGui::Checkbox("EnableFxAA", &_enableFxAA);
				m_ppData.EnableFxAA = _enableFxAA;
			}
		}
		if (ImGui::CollapsingHeader("Particle")) 
		{
			ImGui::Checkbox("Enable particles", &ComputeShaderTest::enableParticle);
			if (ComputeShaderTest::enableParticle)
			{
				ImGui::DragFloat3("Initial Loc min", (float*)&ComputeShaderTest::initialLocMin, 0.1f);
				ImGui::DragFloat3("Initial Loc max", (float*)&ComputeShaderTest::initialLocMax, 0.1f);
				ImGui::DragFloat3("Initial Vel min", (float*)&ComputeShaderTest::initialVelMin, 0.1f);
				ImGui::DragFloat3("Initial Vel max", (float*)&ComputeShaderTest::initialVelMax, 0.1f);

				ImGui::DragFloat("Life time", &ComputeShaderTest::lifeTime, 0.1f, 0.1f, 20.f);
				ImGui::DragFloat("Delay time", &ComputeShaderTest::delayTime, 0.1f, -20.f, 0);
			}
		}
		if (ImGui::Button("Reset"))
		{
			g_renderMode = Graphics::ERM_DeferredShading;
			m_ppData.Exposure = 3.0f;
		}
	}
	ImGui::End();

	ImGui::Begin("SelectedActor");
	if (ISelectable::IsValid(Editor::SelectingItemID))
	{
		ISelectable* _selectable = ISelectable::s_selectableList.at(Editor::SelectingItemID);
		cTransform* _itemTransform = nullptr;
		if (_selectable &&_selectable->GetBoundTransform(_itemTransform))
		{
			float delta = ImGui::GetIO().MouseDelta.x;
			// position
			{
				glm::vec3 _location = _itemTransform->Position();
				if (ImGui::DragFloat3("Location", reinterpret_cast<float*>(&_location), 1.f, -5000.f, 5000.f))
					_itemTransform->SetPosition(_location);
			}
			// rotation
			{
				glm::vec3 _eulerangles = _itemTransform->GetEulerAngle();
				glm::vec3 _prevEulerAngle = _eulerangles;
				glm::quat _quat = _itemTransform->Rotation();

				if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&_eulerangles), 1.f, -3600.f, 3600.f))
				{
					if (glm::abs(_eulerangles.x - _prevEulerAngle.x) > 1.0f)
						_itemTransform->SetRotation(_quat * glm::quat(glm::vec3(glm::radians(delta), 0, 0)));
					else	if (glm::abs(_eulerangles.y - _prevEulerAngle.y) > 1.0f)
						_itemTransform->SetRotation(_quat * glm::quat(glm::vec3(0, glm::radians(delta), 0)));
					else	if (glm::abs(_eulerangles.z - _prevEulerAngle.z) > 1.0f)
						_itemTransform->SetRotation(_quat * glm::quat(glm::vec3(0, 0, glm::radians(delta))));
				}
			}
			// scale
			{
				glm::vec3 _scale = _itemTransform->Scale();
				if (ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&_scale), 0.5f, 0.01f, 1000.f))
				{
					_itemTransform->SetScale(_scale);
				}
			}

			Graphics::cPointLight* _pLight = dynamic_cast<Graphics::cPointLight*>(_selectable);
			if (_pLight)
			{
				if (ImGui::CollapsingHeader("Point Light properties"))
				{
					ImGui::ColorEdit3("Light color", (float*)&_pLight->LightColor);
					ImGui::DragFloat("Light Intensity", &_pLight->Intensity, 0.5f, 0.01f, 1000.f);
					if (ImGui::DragFloat("Light Range", &_pLight->Range, 1.f, 1.f, 1000.f))
					{
						_pLight->Transform.SetScale(glm::vec3(_pLight->Range));
					}
				}
			}

			Graphics::cModel* _model = dynamic_cast<Graphics::cModel*>(_selectable);
			if (_model)
			{
				Graphics::cMatPBRMR* _pbrMat = dynamic_cast<Graphics::cMatPBRMR*>(Graphics::cMaterial::s_manager.Get(_model->GetMaterialAt()));
				if (_pbrMat && ImGui::CollapsingHeader("PBR material properties"))
				{
					ImGui::ColorEdit3("DiffuseColor", reinterpret_cast<float*>(&_pbrMat->DiffuseIntensity));
					ImGui::DragFloat("Metallic", &_pbrMat->MetallicIntensity, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("Roughness", &_pbrMat->RoughnessIntensity, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat3("IoR", reinterpret_cast<float*>(&_pbrMat->IoR), 0.01f, 0.1f, 2.5f);
				}

			}
		}
	}
	else
	{
		ImGui::Text("No actor is selected.");
	}
	ImGui::End();
}

void Assignment::CleanUp()
{
	safe_delete(m_collisionSphere);
	safe_delete(m_editorCamera);
	safe_delete(m_cubemap);
	for (int i = 0; i < g_renderActorList.size(); ++i)
	{
		safe_delete(g_renderActorList[i]);
	}
#ifdef ENABLE_CLOTH_SIM
	ClothSim::CleanUpData();
#endif
	}
