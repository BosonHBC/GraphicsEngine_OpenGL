#include <stdio.h>
#include <string>
#include <fstream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define ToRadian(x) x * 0.0174532925f

const char* s_dataFilePrefix = "Contents/shaders/";
GLuint s_vaoID, s_vboID, s_programID;
GLuint s_iboID;
GLuint s_modelMatrix_UniformID;
// Compile and add shader
// ------------------------------------

// Read shader functions
std::string ReadShaderCode(const char* fileName);

void AddShader(GLuint i_programID, const char* i_shaderName, GLenum i_shaderType);

void CompileShaders();

void CreateTriangle();
void DrawTriangle();
void ApplyTransfomration();

// Function implementations
// ---------------------------------------------------
std::string ReadShaderCode(const char* fileName)
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

void AddShader(GLuint i_programID, const char* i_shaderName, GLenum i_shaderType)
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
		return;
	}

	glAttachShader(i_programID, shaderID);
}

void CompileShaders()
{
	s_programID = glCreateProgram();
	if (!s_programID) {
		printf("Failed to create a shader program");
		return;
	}

	AddShader(s_programID, "vertexShader.glsl", GL_VERTEX_SHADER);
	AddShader(s_programID, "fragmentShader.glsl", GL_FRAGMENT_SHADER);

	// try handle error
	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	// link the program
	glLinkProgram(s_programID);
	glGetProgramiv(s_programID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(s_programID, sizeof(eLog), NULL, eLog);
		printf("Error Linking program: %d \n%s", s_programID, eLog);
		return;
	}

	// validate the program
	glValidateProgram(s_programID);
	glGetProgramiv(s_programID, GL_VALIDATE_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(s_programID, sizeof(eLog), NULL, eLog);
		printf("Error validating program: %d \n%s", s_programID, eLog);
		return;
	}

	// Assign uniform ID
	s_modelMatrix_UniformID = glGetUniformLocation(s_programID, "modelMatrix");


}
// create triangle
void CreateTriangle() {

	GLuint indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1 ,2
	};

	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		0.0, -1.0, 1.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	// VAO
	{
		// generate a VAO
		glGenVertexArrays(1, &s_vaoID);
		// opengl will use this VAO
		glBindVertexArray(s_vaoID);

	}

	// IBO
	{
		glGenBuffers(1, &s_iboID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_iboID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	}

	// VBO
	{
		// generate a VBO
		glGenBuffers(1, &s_vboID);
		// bind this vbo to the vao just created
		glBindBuffer(GL_ARRAY_BUFFER, s_vboID);
		// connect the buffer data(the vertices that just created) to gl array buffer for this vbo
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // static draw, means that this vertices will not change
	}

	// Attribute pointer
	{
		// the location of the pointer points to, in shader layout (location = 0)
		// size of the data that will pass in, in this case, x,y,z is 3
		// the type of the value
		// if normalize the data or not
		// stride means if skip any data, like 3 * GL_FLOAT
		// the offset to start the data
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		// enable the attribute pointer we just created
		glEnableVertexAttribArray(0);
	}

	// unbind VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//unbind IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	// unbind VAO
	glBindVertexArray(0);
}


void ApplyTransfomration()
{
	// model needs to be initialized as identity so that it can translate properly
	glm::mat4 model = glm::identity<glm::mat4>();
	model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
	model = glm::rotate(model, ToRadian(40), glm::vec3(0.0, 1.0, 0.0));
	model = glm::scale(model, glm::vec3(0.4, 0.4, 1));
	// the raw pointer pointer to the model doesn't match opengl shader's requirement, 
	// so we need to use glm::value_ptr to process the raw pointer so that opengl shader can recoginize this pointer
	glUniformMatrix4fv(s_modelMatrix_UniformID, 1, GL_FALSE, glm::value_ptr(model));
}

void DrawTriangle()
{
	// ----------------------
	// Bind program and VAO
	glUseProgram(s_programID);

	ApplyTransfomration();

	// bind VAO
	glBindVertexArray(s_vaoID);

	if(true)
	{
		//bind ibo
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_iboID);
		// Index draw
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
	}
	else
	{
		// Vertex draw
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	
	// clear IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	// clear VAO
	glBindVertexArray(0);
	// clear program
	glUseProgram(0);
}
