#include "GLShader.h"
namespace Graphics
{
	/** inline function for sGLShader */
	std::string sGLShader::ReadShaderCode(const char* i_shaderName)
	{
		std::ifstream meInput(std::string(Constants::CONST_PATH_SHADER_ROOT).append(i_shaderName));
		if (!meInput.good())
		{
			printf("File failed to load [%s]", i_shaderName);
			exit(1);
		}

		return std::string(
			std::istreambuf_iterator<char>(meInput),
			std::istreambuf_iterator<char>());

	}

	void sGLShader::DeleteShader()
	{
		if (IsValid()) {
			glDeleteShader(m_shaderID);
			m_shaderID = GL_INVALID_SHADERID;
		}
	}

	bool sGLShader::CompileShader(const char* i_shaderName, GLenum i_shaderType)
	{
		DeleteShader();

		m_shaderID = glCreateShader(i_shaderType);

		std::string shaderCode = ReadShaderCode(i_shaderName);

		const GLchar* theCode[1];
		theCode[0] = shaderCode.c_str();

		GLint codeLength[1];
		codeLength[0] = shaderCode.length();

		glShaderSource(m_shaderID, 1, theCode, codeLength);
		glCompileShader(m_shaderID);

		// try handle error
		{
			GLint result = 0;
			GLchar eLog[1024] = { 0 };

			glGetShaderiv(m_shaderID, GL_COMPILE_STATUS, &result);
			if (!result) {
				glGetShaderInfoLog(m_shaderID, sizeof(eLog), NULL, eLog);
				printf("Error compiling shader: %s \n%s", i_shaderName, eLog);
				return false;
			}
		}

		return IsValid();
	}

	bool sGLShader::IsValid() const
	{
		return (m_shaderID != GL_INVALID_SHADERID);
	}

	GLuint sGLShader::GetShaderID() const
	{
		return m_shaderID;
	}
}