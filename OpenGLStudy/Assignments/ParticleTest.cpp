#include "ParticleTest.h"
#include "glm/glm.hpp"
#include <time.h>       
#include "Graphics/Effect/Effect.h"
#include "Graphics/Graphics.h"

namespace ComputeShaderTest
{
	GLuint posSSbo;
	GLuint velSSbo;
	GLuint colSSbo;
	GLuint vao;
	GLuint computeShaderProgramID;
	sVel vels[NUM_PARTICLES];
	sPos points[NUM_PARTICLES];

	Graphics::sGLShader* computeShader;

	float randRange(float min, float max)
	{
		//srand(time(NULL));
		double zeroToOne = ((double)rand() / (RAND_MAX));
		return zeroToOne * (max - min) + min;
	}
	const int XMin = -300, XMax = 300, YMin = 200, YMax = 400, ZMin = -150, ZMax = 150;
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

		GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT; // the invalidate makes a big difference when re-writing

		// generate a VAO
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// Position
		{
			glGenBuffers(1, &posSSbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
			
			
			//sPos *points = (sPos *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(sPos), bufMask);
			for (int i = 0; i < NUM_PARTICLES; i++)
			{
				points[i].x = randRange(XMin, XMax);
				points[i].y = randRange(YMin, YMax);
				points[i].z = randRange(ZMin, ZMax);
				points[i].w = 1.;
			}
			//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(sPos), &points[0], GL_DYNAMIC_DRAW);
		}
		// velocity
		{
			glGenBuffers(1, &velSSbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
			//glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(sVel), NULL, GL_DYNAMIC_DRAW);

			//sVel *vels = (sVel *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(sVel), bufMask);
			
			for (int i = 0; i < NUM_PARTICLES; i++)
			{
				vels[i].vx = randRange(-10, 10);
				vels[i].vy = randRange(-10, 10);
				vels[i].vz = randRange(-10, 10);
				vels[i].vw = 0;
			}
			glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(sVel), &vels[0], GL_DYNAMIC_DRAW);
			//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
/*
		// color
		{
			glGenBuffers(1, &colSSbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, colSSbo);
			glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(sColor), NULL, GL_STATIC_DRAW);

			sColor *cols = (sColor *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(sColor), bufMask);
			for (int i = 0; i < NUM_PARTICLES; i++)
			{
				cols[i].r = randRange(0.1f, 1.f);
				cols[i].g = randRange(0.1f, 1.f);
				cols[i].b = randRange(0.1f, 1.f);
				cols[i].a = 1;
			}
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}*/

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
		glDeleteBuffers(1, &colSSbo);
		assert(GL_NO_ERROR == glGetError());
	}

	void RenderParticle()
	{
		// compute shader stage
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, posSSbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, velSSbo);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, colSSbo);
		assert(GL_NO_ERROR == glGetError());

		glUseProgram(computeShaderProgramID);
		glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		Graphics::cEffect* particleEffect = Graphics::GetEffectByKey(Graphics::EET_ParticleTest);
		particleEffect->UseEffect();
		
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, posSSbo);
		assert(GL_NO_ERROR == glGetError());

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		assert(GL_NO_ERROR == glGetError());

		glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
		assert(GL_NO_ERROR == glGetError());

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);
	
		particleEffect->UnUseEffect();

	}

}