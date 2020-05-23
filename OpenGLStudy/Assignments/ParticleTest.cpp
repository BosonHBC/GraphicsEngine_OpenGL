#include "ParticleTest.h"
#include "glm/glm.hpp"
#include <time.h>       
#include "Graphics/Effect/Effect.h"
#include "Graphics/Graphics.h"

#include "Application/Application.h"

namespace ComputeShaderTest
{
	GLuint posSSbo;
	GLuint velSSbo;
	GLuint dataSSbo;
	GLuint vao;
	GLuint computeShaderProgramID;
	sVel vels[NUM_PARTICLES];
	sPos points[NUM_PARTICLES];

	GLuint timeUniformID;
	GLuint lifeTimeUniformID, delayTimeUniformID;
	GLuint initLocMinID, initLocMaxID, initVelMinID, initVelMaxID;

	float lifeTime = 5, delayTime = -5;
	glm::vec3 initialLocMin(-10,150, -10), initialLocMax(10, 160, 10), initialVelMin(-100, 0, -100), initialVelMax(100, 100, 100);
	Graphics::sGLShader* computeShader;

	float randRange(float min, float max)
	{
		//srand(time(NULL));
		double zeroToOne = ((double)rand() / (RAND_MAX));
		return static_cast<float>((zeroToOne * (max - min) + min));
	}
	bool Init()
	{
		// create compute program
		{
			computeShaderProgramID = glCreateProgram();

			computeShader = new Graphics::sGLShader();
			const char* path = "particle/particle_comp.glsl";
			if (!computeShader->CompileShader(path, GL_COMPUTE_SHADER)) {
				printf("Error compiling shader[%s]\n", path);
				assert(false);
			}

			glAttachShader(computeShaderProgramID, computeShader->GetShaderID());

			// link program
			GLint result = 0;
			GLchar eLog[1024] = { 0 };
			glLinkProgram(computeShaderProgramID);
			glGetProgramiv(computeShaderProgramID, GL_LINK_STATUS, &result);
			if (!result) {
				glGetProgramInfoLog(computeShaderProgramID, sizeof(eLog), NULL, eLog);
				auto errorCode = glGetError();
				printf("Error Linking program: %d, error message: %s, error code: %d\n", computeShaderProgramID, eLog, errorCode);
				assert(false);
			}
			assert(GL_NO_ERROR == glGetError());

			// validate the program
			glValidateProgram(computeShaderProgramID);
			// try handle error
			glGetProgramiv(computeShaderProgramID, GL_VALIDATE_STATUS, &result);
			if (!result) {
				glGetProgramInfoLog(computeShaderProgramID, sizeof(eLog), NULL, eLog);
				printf("Error validating program: %d \n%s", computeShaderProgramID, eLog);
				assert(false);
			}
		}
		// set up uniform variables
		{
			timeUniformID = glGetUniformLocation(computeShaderProgramID, "g_time");
			lifeTimeUniformID = glGetUniformLocation(computeShaderProgramID, "g_lifeTime");
			delayTimeUniformID = glGetUniformLocation(computeShaderProgramID, "g_delayTime"); 
			initLocMinID = glGetUniformLocation(computeShaderProgramID, "g_initialLocMin");
			initLocMaxID = glGetUniformLocation(computeShaderProgramID, "g_initialLocMax");
			initVelMinID = glGetUniformLocation(computeShaderProgramID, "g_initialVelMin");
			initVelMaxID = glGetUniformLocation(computeShaderProgramID, "g_initialVelMax");
		}
		GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT; // the invalidate makes a big difference when re-writing

		// generate a VAO
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// Position
		{
			glGenBuffers(1, &posSSbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
			glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(sPos), NULL, GL_STATIC_DRAW);

			sPos *points = (sPos *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(sPos), bufMask);
			for (int i = 0; i < NUM_PARTICLES; i++)
			{
				points[i].x = randRange(initialLocMin.x, initialLocMax.x);
				points[i].y = randRange(initialLocMin.y, initialLocMax.y);
				points[i].z = randRange(initialLocMin.z, initialLocMax.z);
				points[i].w = 1.;
			}
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			
		}
		// velocity
		{
			glGenBuffers(1, &velSSbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
			//glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(sVel), NULL, GL_DYNAMIC_DRAW);

			//sVel *vels = (sVel *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(sVel), bufMask);
			
			for (int i = 0; i < NUM_PARTICLES; i++)
			{
				vels[i].vx = randRange(initialVelMin.x, initialVelMax.x);
				vels[i].vy = randRange(initialVelMin.y, initialVelMax.y);
				vels[i].vz = randRange(initialVelMin.z, initialVelMax.z);
				vels[i].vw = 0;
			}
			glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(sVel), &vels[0], GL_DYNAMIC_DRAW);
			//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}

		// color
		{
			glGenBuffers(1, &dataSSbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, dataSSbo);
			glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(sOtherData), NULL, GL_STATIC_DRAW);

			sOtherData *datas = (sOtherData *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(sOtherData), bufMask);
			for (int i = 0; i < NUM_PARTICLES; i++)
			{
				datas[i].elpasedLifeTime = randRange(delayTime, 0);
			}
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}

		glBindVertexArray(0);
		return true;
	}

	void cleanUp()
	{
		safe_delete(computeShader);
		glDeleteProgram(computeShaderProgramID);
		computeShaderProgramID = 0;
		glDeleteBuffers(1, &posSSbo);
		glDeleteBuffers(1, &velSSbo);
		glDeleteBuffers(1, &dataSSbo);
		assert(GL_NO_ERROR == glGetError());
	}

	void RenderParticle()
	{
		// compute shader stage
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, posSSbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, velSSbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, dataSSbo);
		assert(GL_NO_ERROR == glGetError());

		glUseProgram(computeShaderProgramID);
		Application::cApplication* currentApp = Application::GetCurrentApplication();
		float elpasedTime = currentApp->GetSystemElapsedTime();
		glUniform1f(timeUniformID, elpasedTime);
		glUniform1f(lifeTimeUniformID, lifeTime);
		glUniform1f(delayTimeUniformID, delayTime);

		glUniform3f(initLocMinID,initialLocMin.x, initialLocMin.y, initialLocMin.z);
		glUniform3f(initLocMaxID, initialLocMax.x, initialLocMax.y, initialLocMax.z);
		glUniform3f(initVelMinID, initialVelMin.x, initialVelMin.y, initialVelMin.z);
		glUniform3f(initVelMaxID, initialVelMax.x, initialVelMax.y, initialVelMax.z);

		glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		Graphics::cEffect* particleEffect = Graphics::GetEffectByKey(Graphics::EET_ParticleTest);
		particleEffect->UseEffect();
		
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, posSSbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, dataSSbo);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
		assert(GL_NO_ERROR == glGetError());

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	
		particleEffect->UnUseEffect();

	}

}