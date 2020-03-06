#include "Effect.h"
#include <string>
#include <stdio.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader/GLShader.h"

namespace Graphics {
	cEffect::cEffect()
	{
		m_programID = 0;
	}

	cEffect::~cEffect()
	{
		CleanUp();
	}

	bool cEffect::CreateProgram(const char* const i_vertexShaderPath, const char* const i_fragmentShaderPath, const char* const i_geometryShaderPath)
	{
		m_programID = glCreateProgram();
		if (!m_programID) {
			printf("Failed to create a shader program\n");
			return false;
		}

		if (!LoadShader(i_vertexShaderPath, GL_VERTEX_SHADER)) {
			printf("Can not create program without vertex shader\n");
			return false;
		}
		if (!LoadShader(i_fragmentShaderPath, GL_FRAGMENT_SHADER))
		{
			printf("Can not create program without fragment shader\n");
			return false;
		}

		// link the program
		if (!LinkProgram()) {
			return false;
		}

		if (!BindUniformVariables()) {
			printf("Error binding uniform variables!");
			return false;
		}
		return true;
	}

	bool cEffect::LinkProgram()
	{
		GLint result = 0;
		GLchar eLog[1024] = { 0 };
		glLinkProgram(m_programID);
		glGetProgramiv(m_programID, GL_LINK_STATUS, &result);
		if (!result) {
			glGetProgramInfoLog(m_programID, sizeof(eLog), NULL, eLog);
			printf("Error Linking program: %d \n%s", m_programID, eLog);
			return false;
		}
		return true;
	}

	bool cEffect::ValidateProgram()
	{
		GLint result = 0;
		GLchar eLog[1024] = { 0 };
		// validate the program
		glValidateProgram(m_programID);
		// try handle error
		glGetProgramiv(m_programID, GL_VALIDATE_STATUS, &result);
		if (!result) {
			glGetProgramInfoLog(m_programID, sizeof(eLog), NULL, eLog);
			printf("Error validating program: %d \n%s", m_programID, eLog);
			return false;
		}
		return true;
	}

	void cEffect::FixSamplerError()
	{
		UseEffect();
		glUniform1i(glGetUniformLocation(m_programID, "cubemapTex"), 3);
		UnUseEffect();
		assert(glGetError() == GL_NO_ERROR);
	}

	void cEffect::CleanUp()
	{
		// Clean up GLShader
		for (auto it = m_shaders.begin(); it != m_shaders.end(); ++it)
		{
			delete it->second;
			it->second = nullptr;
		}
		m_shaders.clear();

		if (m_programID != 0)
		{
			glDeleteProgram(m_programID);
			m_programID = 0;
		}

	}

	bool cEffect::BindUniformVariables()
	{
		return true;
	}

	void cEffect::UseEffect()
	{
		// Bind program
		glUseProgram(m_programID);

	}

	void cEffect::UnUseEffect()
	{
		glUseProgram(0);
	}

	bool cEffect::RecompileShader(const char* i_shaderName, GLenum i_shaderType)
	{
		//CreateProgram(Constants::CONST_PATH_DEFAULT_VERTEXSHADER, Constants::CONST_PATH_BLINNPHONG_FRAGMENTSHADER);
		return true;
	}

	bool cEffect::LoadShader(const char* i_shaderName, GLenum i_shaderType)
	{
		// this type of shader did not exist in GL context
		if (m_shaders.count(i_shaderType) <= 0) {
			sGLShader* _newShader = new sGLShader();
			if (!_newShader->CompileShader(i_shaderName, i_shaderType)) {
				printf("Error compiling shader[%s]\n", i_shaderName);
				return false;
			}
			m_shaders.insert({ i_shaderType, _newShader });
		}
		else {
			// if this shader already existed in this effect, need to recompile it
			if (!m_shaders.at(i_shaderType)->CompileShader(i_shaderName, i_shaderType)) {
				printf("Error compiling shader[%s]\n", i_shaderName);
				return false;
			}
		}

		// Attach this shader to this program
		glAttachShader(m_programID, m_shaders.at(i_shaderType)->GetShaderID());
		return true;
	}

}
