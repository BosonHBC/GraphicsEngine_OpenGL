#pragma once

#include "GL/glew.h"
#include <map>
#include "glm/gtc/matrix_transform.hpp"
namespace Graphics {
	/** Forward declaration*/
	struct sGLShader;
	// cEffect represent the openGL program, contains aggregation of shaders
	class cEffect
	{
	public:
		/** Constructors and destructor */
		cEffect();
		~cEffect();

		/** Initializations and clean up*/
		/** Create program with default vertex shader and fragment shader*/
		bool CreateProgram(const char* const i_vertexShaderPath, const char* const i_fragmentShaderPath, const char* const i_geometryShaderPath = "");
		bool LinkProgram();
		bool ValidateProgram();
		void FixSamplerError();
		void CleanUp();

		/** Usage functions*/
		void UseEffect();
		void UnUseEffect();
		bool RecompileShader(const char* i_shaderName, GLenum i_shaderType);

		/** Getters */
		const GLuint& GetProgramID() const { return m_programID; }
	protected:
		/** private variables*/
		GLuint m_programID;

		std::map<GLenum, sGLShader*> m_shaders;

		/** private helper functions*/
		// Initialize shaders
		bool LoadShader(const char* i_shaderName, GLenum i_shaderType);
		// Find the variable ids
		virtual bool BindUniformVariables();

	};

}
