#include "Effect.h"
#include <string>
#include <fstream>
#include <stdio.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

const char* s_dataFilePrefix = "Contents/shaders/";

namespace Graphics {
	cEffect::cEffect()
	{
		m_programID = 0;
		m_modelMatrixID = 0;
		m_projectionMatrixID = 0;
	}

	cEffect::~cEffect()
	{
		CleanUp();
	}

	bool cEffect::CreateProgram()
	{
		m_programID = glCreateProgram();
		if (!m_programID) {
			printf("Failed to create a shader program\n");
			return false;
		}

		if (!LoadShader("vertexShader.glsl", GL_VERTEX_SHADER)) {
			printf("Can not create program without vertex shader\n");
			return false;
		}
		if (!LoadShader("fragmentShader.glsl", GL_FRAGMENT_SHADER))
		{
			printf("Can not create program without fragment shader\n");
			return false;
		}

		// try handle error
		GLint result = 0;
		GLchar eLog[1024] = { 0 };

		// link the program
		glLinkProgram(m_programID);
		glGetProgramiv(m_programID, GL_LINK_STATUS, &result);
		if (!result) {
			glGetProgramInfoLog(m_programID, sizeof(eLog), NULL, eLog);
			printf("Error Linking program: %d \n%s", m_programID, eLog);
			return false;
		}

		// validate the program
		glValidateProgram(m_programID);
		glGetProgramiv(m_programID, GL_VALIDATE_STATUS, &result);
		if (!result) {
			glGetProgramInfoLog(m_programID, sizeof(eLog), NULL, eLog);
			printf("Error validating program: %d \n%s", m_programID, eLog);
			return false;
		}

		if (!BindUniformVariables()) {
			printf("Error binding uniform variables!");
			return false;
		}
		return true;
	}

	void cEffect::CleanUp()
	{
		if (m_programID != 0)
		{
			glDeleteProgram(m_programID);
			m_programID = 0;
		}
		m_modelMatrixID = 0;
		m_projectionMatrixID = 0;
	}

	bool cEffect::BindUniformVariables()
	{
		// Assign uniform ID
		m_modelMatrixID = glGetUniformLocation(m_programID, "modelMatrix");
		m_projectionMatrixID = glGetUniformLocation(m_programID, "projectionMatrix");

		return true;
	}

	void cEffect::UseEffect()
	{
		// Bind program and VAO
		glUseProgram(m_programID);
	}

	bool cEffect::LoadShader(const char* i_shaderName, GLenum i_shaderType)
	{
		GLuint shaderID = glCreateShader(i_shaderType);

		std::string shaderCode = ReadShaderCode(i_shaderName);

		const GLchar* theCode[1];
		theCode[0] = shaderCode.c_str();

		GLint codeLength[1];
		codeLength[0] = shaderCode.length();

		glShaderSource(shaderID, 1, theCode, codeLength);
		glCompileShader(shaderID);

		// try handle error
		GLint result = 0;
		GLchar eLog[1024] = { 0 };

		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
		if (!result) {
			glGetShaderInfoLog(shaderID, sizeof(eLog), NULL, eLog);
			printf("Error compiling shader: %s \n%s", i_shaderName, eLog);
			return false;
		}

		glAttachShader(m_programID, shaderID);

		return true;
	}

	std::string cEffect::ReadShaderCode(const char* fileName)
	{
		std::ifstream meInput(std::string(s_dataFilePrefix).append(fileName));
		if (!meInput.good())
		{
			printf("File failed to load [%s]", fileName);
			exit(1);
		}

		return std::string(
			std::istreambuf_iterator<char>(meInput),
			std::istreambuf_iterator<char>());

	}

}
