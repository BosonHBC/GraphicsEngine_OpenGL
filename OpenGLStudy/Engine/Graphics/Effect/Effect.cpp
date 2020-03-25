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

	bool cEffect::CreateProgram(const char* const i_vertexShaderPath, const char* const i_fragmentShaderPath, const char* const i_geometryShaderPath /*= ""*/, const char* const i_TCSPath /*= ""*/, const char* const i_TESPath/* = ""*/)
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

		if (!IsPathNull(i_geometryShaderPath))
		{
			if (!LoadShader(i_geometryShaderPath, GL_GEOMETRY_SHADER))
			{
				printf("Can not create program because of failing compiling geometry shader. \n");
				return false;
			}
		}
		if (!IsPathNull(i_TCSPath))
		{
			if (!LoadShader(i_TCSPath, GL_TESS_CONTROL_SHADER))
			{
				printf("Can not create program because of failing compiling TCS shader. \n");
				return false;
			}
		}
		if (!IsPathNull(i_TESPath))
		{
			if (!LoadShader(i_TESPath, GL_TESS_EVALUATION_SHADER))
			{
				printf("Can not create program because of failing compiling TCS shader. \n");
				return false;
			}
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

	bool cEffect::IsPathNull(const char* const i_incomingPath)
	{
		if ((i_incomingPath != nullptr) && (i_incomingPath[0] == '\0')) {
			return true;
		}
		return false;
	}

	bool cEffect::IsUniformIDValid(const GLuint& i_id)
	{
		return i_id >= 0 && i_id < static_cast<GLuint>(-1);
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

		return true;
	}

	void cEffect::SetInteger(const char* const i_uniformName, const GLint& i_int)
	{
		GLuint _ID = glGetUniformLocation(m_programID, i_uniformName);
		if (IsUniformIDValid(_ID))
			glUniform1i(_ID, i_int);
		assert(GL_NO_ERROR == glGetError());
	}

	void cEffect::SetFloat(const char* const i_uniformName, const GLfloat& i_float)
	{
		GLuint _ID = glGetUniformLocation(m_programID, i_uniformName);
		if(IsUniformIDValid(_ID)) glUniform1f(_ID, i_float);
		
		assert(GL_NO_ERROR == glGetError());
	}

	void cEffect::SetVec3(const char* const i_uniformName, const glm::vec3& i_vec3)
	{
		GLuint _ID = glGetUniformLocation(m_programID, i_uniformName);
		if (IsUniformIDValid(_ID)) glUniform3f(_ID, i_vec3.x, i_vec3.y, i_vec3.z);

		assert(GL_NO_ERROR == glGetError());
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
