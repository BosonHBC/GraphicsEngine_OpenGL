#pragma once
#include "GL/glew.h"
#include "Constants/Constants.h"
#include "string"
#include <fstream>
#ifndef GL_INVALID_SHADERID
#define GL_INVALID_SHADERID -1
#endif
namespace Graphics {
	struct sGLShader
	{
		// default constructor
		sGLShader() : m_shaderID(GL_INVALID_SHADERID) {}
		~sGLShader() { DeleteShader(); }
		bool CompileShader(const char* i_shaderName, GLenum i_shaderType);
		bool IsValid() const;
		GLuint GetShaderID() const;

	private:
		/** The shaderID given by OpenGL*/
		GLuint m_shaderID;
		// Read shader by files
		std::string ReadShaderCode(const char* i_shaderName);
		// Remove this shader from openGL context
		void DeleteShader();
	};

	

}