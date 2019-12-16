#pragma once
#include <string>
#include "GL/glew.h"

#include "glm/gtc/matrix_transform.hpp"
namespace Graphics {
	// cEffect represent the openGL program, contains aggregation of shaders
	class cEffect
	{
	public:
		/** Constructors and destructor */
		cEffect();
		~cEffect();

		/** Initializations and clean up*/
		bool CreateProgram();
		void CleanUp();

		// Use the effect
		void UseEffect();



		/** Getters */
		const GLuint& GetModelMatrixUniformID() const { return m_modelMatrixID; }
		const GLuint& GetProjectionMatrixUniformID() const { return m_projectionMatrixID; }
		const GLuint& GetProgramID() const { return m_programID; }
	private:
		GLuint m_programID;
		GLuint m_modelMatrixID, m_projectionMatrixID;


		// Initialize shaders
		bool LoadShader(const char* i_shaderName, GLenum i_shaderType);
		// Read shader by files
		std::string ReadShaderCode(const char* i_shaderName);
		// Find the variable ids
		virtual bool BindUniformVariables();

	};

}
