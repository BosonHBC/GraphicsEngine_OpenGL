#include <stdio.h>
#include <mutex>
#include "Window.h"
#include "WindowInput.h"

#include "Application/imgui/imgui.h"
#include "Application/imgui/imgui_impl_glfw.h"
#include "Application/imgui/imgui_impl_opengl3.h"

cWindow::~cWindow()
{
	CleanUp();
}

bool cWindow::Initialzation()
{
	//00. Init GLFW
	{
		if (!glfwInit()) {
			printf("GLFW initialization failed!");
			glfwTerminate();
			return false;
		}
	}

	//01. Create window
	{
		if (!SetupGLFWWindow(m_glfwWindow, m_windowName)) return false;
	}

	// 1.5 Setup IMGUI
	{
		if (!SetupImGUI()) return false;
	}
	//02. Set buffer size information
	{
		glfwGetFramebufferSize(m_glfwWindow, &m_bufferWidth, &m_bufferHeight);
	}

	//03.Set context for GLFW to use, let opengl know that which window is going to be drawn in
	{
		// By calling this function, we can switch between different windows
		glfwMakeContextCurrent(m_glfwWindow);
	}

	//04. InitializeGL GLEW
	{
		//Allow modern extension features, allow us to use modern extension, which can make things easier
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK) {
			glfwDestroyWindow(m_glfwWindow);
			glfwTerminate();
			printf("GLEW Initialization failed!");
			return false;
		}
	}

	//05. Set up viewport, which part of the window will draw things
	{
		SetViewportSize(m_bufferWidth, m_bufferHeight);
	}

	//06. enable features
	{
		glEnable(GL_DEPTH_TEST);
	}

	//07. setup input callback
	{
		// Assign user pointer to this window, so that GLFW window knows this cWindow reference so that we can get keys variable
		glfwSetWindowUserPointer(m_glfwWindow, this);
		CreateCallbacks();
		// Set input mode
		glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	return true;
}

void cWindow::CleanUp()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	if (m_windowInput) {
		delete m_windowInput;
		m_windowInput = nullptr;
	}
	if (m_glfwWindow) {
		glfwDestroyWindow(m_glfwWindow);
	}
	glfwTerminate();
}

bool cWindow::GetShouldClose() const
{
	return glfwWindowShouldClose(m_glfwWindow);
}

bool cWindow::SetupGLFWWindow(GLFWwindow*& o_mainWindow, const char* i_windowName, GLFWmonitor* i_monitor /*= nullptr*/, GLFWwindow* i_sharedWindow /*= nullptr*/)
{
	// Set the major version of this window to 3, since we are using 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Core profile = No backwards compatibility
	// This line will throw an error when we are trying to use old features, function which might be removed in the future
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Allow forward compatibility
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	o_mainWindow = glfwCreateWindow(m_bufferWidth, m_bufferHeight, i_windowName, i_monitor, i_sharedWindow);
	if (!o_mainWindow) {
		printf("Create GLFW window failed!");
		glfwTerminate();
		return false;
	}
	return true;
}

bool cWindow::SetupImGUI()
{
	auto result = true;
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	if (!(result = ImGui_ImplGlfw_InitForOpenGL(m_glfwWindow, true)))
	{
		printf("Fail to Init glfw for ImGUI.\n");
		return result;
	}
		
	const char* glsl_version = "#version 430";
	if (!(result = ImGui_ImplOpenGL3_Init(glsl_version)))
	{
		printf("Fail to Init opengl3 for ImGUI.\n");
		return result;
	}
}

void cWindow::CreateCallbacks()
{
	// According to glfw website: https://www.glfw.org/docs/latest/group__keys.html
	// Supported key numbers are 350+, so I set it to 360
	m_windowInput = new sWindowInput(GLFW_MAX_KEY_COUNT);
	glfwSetCursorPosCallback(m_glfwWindow, &cWindow::HandleMouse);
	glfwSetMouseButtonCallback(m_glfwWindow, &cWindow::HandleMouseButton);
	glfwSetWindowSizeCallback(m_glfwWindow, &cWindow::OnWindowSizeChanged);
}

void cWindow::HandleKeys(GLFWwindow* i_window, int i_key, int i_code, int i_action, int i_mode)
{
	// Get reference to the instanced window
	cWindow* thisWindow = static_cast<cWindow*>(glfwGetWindowUserPointer(i_window));
	sWindowInput* _input = thisWindow->m_windowInput;

	if (i_key == GLFW_KEY_ESCAPE && i_action == GLFW_PRESS) {
		// tell glfw window that it is time to close
		glfwSetWindowShouldClose(i_window, GL_TRUE);
	}

}

void cWindow::HandleMouse(GLFWwindow* i_window, double i_xPos, double i_yPos)
{
	// Get reference to the instanced window
	cWindow* thisWindow = static_cast<cWindow*>(glfwGetWindowUserPointer(i_window));
	sWindowInput* _input = thisWindow->m_windowInput;

	
	if (_input->isFirstMove) {
		_input->lastX = static_cast<GLfloat>(i_xPos);
		_input->lastY = static_cast<GLfloat>(i_yPos);
		_input->isFirstMove = false;
	}

	_input->dx = static_cast<GLfloat>(i_xPos - _input->lastX);
	// prevent inverted control in Y axis
	_input->dy = static_cast<GLfloat>(_input->lastY - i_yPos);

	_input->lastX = static_cast<GLfloat>(i_xPos);
	_input->lastY = static_cast<GLfloat>(i_yPos);
}

void cWindow::HandleMouseButton(GLFWwindow* i_window, int i_button, int i_action, int i_mods)
{
	// Get reference to the instanced window
	cWindow* thisWindow = static_cast<cWindow*>(glfwGetWindowUserPointer(i_window));
	sWindowInput* _input = thisWindow->m_windowInput;

	if (i_button >= 0 && i_button <= GLFW_MAX_BUTTON_COUNT) {

		if (i_action == GLFW_PRESS) {
			_input->SetButton(i_button, true);
		}
		else if (i_action == GLFW_RELEASE) {
			_input->SetButton(i_button, false);
		}
	}

}

void cWindow::OnWindowSizeChanged(GLFWwindow* window, int width, int height)
{
	cWindow* thisWindow = static_cast<cWindow*>(glfwGetWindowUserPointer(window));
	thisWindow->m_bufferWidth = width;
	thisWindow->m_bufferHeight = height;

}

void cWindow::SwapBuffers()
{
	glfwSwapBuffers(m_glfwWindow);
}

void cWindow::SetViewportSize(GLuint i_newWidth, GLuint i_newHeight)
{
	glViewport(0, 0, i_newWidth, i_newHeight);

}

void cWindow::SetViewPort(const sRect& i_newViewPort)
{
	glViewport(i_newViewPort.Min.x, i_newViewPort.Min.y, i_newViewPort.w(), i_newViewPort.h());
}
