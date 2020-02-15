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
		bool CreateProgram(const char* const i_vertexShaderPath, const char* const i_fragmentShaderPath);
		void CleanUp();

		/** Usage functions*/
		void UseEffect();
		void SetPointLightCount(int i_pointLightCount);
		void SetSpotLightCount(int i_spotLightCount);

		bool RecompileShader(const char* i_shaderName, GLenum i_shaderType);

		/** Getters */
		const GLuint& GetProgramID() const { return m_programID; }
	protected:
		/** private variables*/
		GLuint m_programID;
		GLuint m_pointLightCountID, m_spotLightCountID;

		std::map<GLenum, sGLShader*> m_shaders;

		/** private helper functions*/
		// Initialize shaders
		bool LoadShader(const char* i_shaderName, GLenum i_shaderType);
		// Find the variable ids
		virtual bool BindUniformVariables();

	};

}
