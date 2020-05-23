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
	sVel vels[NUM_PARTICLES];
	sPos points[NUM_PARTICLES];

	float lifeTime = 5, delayTime = -5;
	glm::vec3 initialLocMin(-10, 150, -10), initialLocMax(10, 160, 10), initialVelMin(-100, 0, -100), initialVelMax(100, 100, 100);
	bool enableParticle = false;

	float randRange(float min, float max)
	{
		//srand(time(NULL));
		double zeroToOne = ((double)rand() / (RAND_MAX));
		return static_cast<float>((zeroToOne * (max - min) + min));
	}
	bool Init()
	{
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
			glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(sVel), NULL, GL_STATIC_DRAW);

			sVel *vels = (sVel *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(sVel), bufMask);

			for (int i = 0; i < NUM_PARTICLES; i++)
			{
				vels[i].vx = randRange(initialVelMin.x, initialVelMax.x);
				vels[i].vy = randRange(initialVelMin.y, initialVelMax.y);
				vels[i].vz = randRange(initialVelMin.z, initialVelMax.z);
				vels[i].vw = 0;
			}

			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		// other data
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
		glDeleteBuffers(1, &posSSbo);
		glDeleteBuffers(1, &velSSbo);
		glDeleteBuffers(1, &dataSSbo);
		assert(GL_NO_ERROR == glGetError());
	}

	void RenderParticle()
	{
		if (!enableParticle) return;
		// compute shader stage
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, posSSbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, velSSbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, dataSSbo);
		assert(GL_NO_ERROR == glGetError());

		Graphics::cEffect* compEffect = Graphics::GetEffectByKey(Graphics::EET_Comp_Particle);
		compEffect->UseEffect();
		Application::cApplication* currentApp = Application::GetCurrentApplication();
		float elpasedTime = currentApp->GetSystemElapsedTime();
		compEffect->SetFloat("g_time", elpasedTime);
		compEffect->SetFloat("g_lifeTime", lifeTime);
		compEffect->SetFloat("g_delayTime", delayTime);

		compEffect->SetVec3("g_initialLocMin", initialLocMin);
		compEffect->SetVec3("g_initialLocMax", initialLocMax);
		compEffect->SetVec3("g_initialVelMin", initialVelMin);
		compEffect->SetVec3("g_initialVelMax", initialVelMax);

		glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		compEffect->UnUseEffect();

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

	void ResetParticles()
	{

	}
}