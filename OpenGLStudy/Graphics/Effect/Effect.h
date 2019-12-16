#pragma once
#include <string>
#include "GL/glew.h"

#include "glm/gtc/matrix_transform.hpp"

class Effect
{
public:
	Effect();
	~Effect();
	
	// Create programs by adding shaders
	bool CreateProgram();
	// Create projection matrix
	void CreateProjectionMatrix(GLfloat i_fov, GLfloat i_aspect, GLfloat i_nearPlane = 0.1f, GLfloat i_farPlane = 100.f);
	
	// Use the effect
	void UseEffect();
	
	// Clean up function
	void CleanUp();

	// Getters
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
