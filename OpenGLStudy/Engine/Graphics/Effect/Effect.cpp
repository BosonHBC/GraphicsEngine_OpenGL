#include "Effect.h"
#include <string>
#include <stdio.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader/GLShader.h"
#include "Cores/Core.h"
#include "Constants/Constants.h"
#include "Graphics/EnvironmentCaptureManager.h"

namespace Graphics {
	cEffect::cEffect()
	{
		m_programID = 0;
	}

	cEffect::~cEffect()
	{
		CleanUp();
	}

	bool cEffect::CreateProgram(eEffectType i_effectType, const char* const i_vertexShaderPath, const char* const i_fragmentShaderPath, const char* const i_geometryShaderPath /*= ""*/, const char* const i_TCSPath /*= ""*/, const char* const i_TESPath/* = ""*/, const char* const i_ComputePath /*= ""*/)
	{
		m_programID = glCreateProgram();
		if (!m_programID) {
			printf("Failed to create a shader program\n");
			return false;
		}
		// if this is not a compute shader
		if (IsPathNull(i_ComputePath))
		{
			if (!LoadShader(i_vertexShaderPath, GL_VERTEX_SHADER)) {
				printf("Can not create program without vertex shader\n");
				return false;
			}
			if (!LoadShader(i_fragmentShaderPath, GL_FRAGMENT_SHADER))
			{
				printf("Can not create program without fragment shader\n");
				return false;
			}

			if (!IsPathNull(i_geometryShaderPath))
			{
				if (!LoadShader(i_geometryShaderPath, GL_GEOMETRY_SHADER))
				{
					printf("Can not create program because of failing compiling geometry shader. \n");
					return false;
				}
			}
			if (!IsPathNull(i_TCSPath))
			{
				if (!LoadShader(i_TCSPath, GL_TESS_CONTROL_SHADER))
				{
					printf("Can not create program because of failing compiling TCS shader. \n");
					return false;
				}
			}
			if (!IsPathNull(i_TESPath))
			{
				if (!LoadShader(i_TESPath, GL_TESS_EVALUATION_SHADER))
				{
					printf("Can not create program because of failing compiling TCS shader. \n");
					return false;
				}
			}
		}
		else
		{
			if (!LoadShader(i_ComputePath, GL_COMPUTE_SHADER)) {
				printf("Can not create a program with invalid compute shader \n");
				return false;
			}
		}
		// link the program
		if (!LinkProgram()) {
			return false;
		}

		m_type = i_effectType;
		if (!BindUniformVariables()) {
			printf("Error binding uniform variables!");
			return false;
		}


		return true;
	}

	bool cEffect::LinkProgram()
	{
		GLint result = 0;
		GLchar eLog[1024] = { 0 };
		glLinkProgram(m_programID);
		glGetProgramiv(m_programID, GL_LINK_STATUS, &result);
		if (!result) {
			glGetProgramInfoLog(m_programID, sizeof(eLog), NULL, eLog);
			printf("Error Linking program: %d \n%s", m_programID, eLog);
			return false;
		}
		assert(GL_NO_ERROR == glGetError());
		return true;
	}

	bool cEffect::ValidateProgram()
	{
		GLint result = 0;
		GLchar eLog[1024] = { 0 };
		// validate the program
		glValidateProgram(m_programID);
		// try handle error
		glGetProgramiv(m_programID, GL_VALIDATE_STATUS, &result);
		if (!result) {
			glGetProgramInfoLog(m_programID, sizeof(eLog), NULL, eLog);
			printf("Error validating program: %d \n%s", m_programID, eLog);
			return false;
		}
		return true;
	}

	void cEffect::CleanUp()
	{
		// Clean up GLShader
		for (auto it = m_shaders.begin(); it != m_shaders.end(); ++it)
		{
			delete it->second;
			it->second = nullptr;
		}
		m_shaders.clear();

		if (m_programID != 0)
		{
			glDeleteProgram(m_programID);
			m_programID = 0;
		}

	}

	bool cEffect::BindUniformVariables()
	{
		UseEffect();
		switch (m_type)
		{
		case Graphics::EET_BlinnPhong:
			FixSamplerProblem();
			break;
		case Graphics::EET_ShadowMap:
			break;
		case Graphics::EET_OmniShadowMap:
			break;
		case Graphics::EET_Cubemap:
			break;
		case Graphics::EET_Unlit:
			break;
		case Graphics::EET_NormalDisplay:
			break;
		case Graphics::EET_PBR_MR:
			FixSamplerProblem();
			break;
		case Graphics::EET_HDRToCubemap:
			SetInteger("rectangularHDRMap", 0);
			break;
		case Graphics::EET_IrradConvolution:
			break;
		case Graphics::EET_CubemapPrefilter:
			break;
		case Graphics::EET_BrdfIntegration:
			break;
		case Graphics::EET_DrawDebugCircles:
			break;
		case Graphics::EET_TessQuad:
			FixSamplerProblem();
			break;
		case Graphics::EET_TriangulationDisplay:
			break;
		case Graphics::EET_HDREffect:
			SetInteger("hdrBuffer", 0);
			SetInteger("enableHDR", true);
			break;
		case Graphics::EET_GBuffer:
			break;
		case Graphics::EET_GBufferDisplay:
			SetInteger("gAlbedoMetallic", 0);
			SetInteger("gNormalRoughness", 1);
			SetInteger("gIOR", 2);
			SetInteger("gDepth", 3);
			SetInteger("gSSAOMap", 4);
			break;
		case Graphics::EET_DeferredLighting:
			SetInteger("gAlbedoMetallic", 0);
			SetInteger("gNormalRoughness", 1);
			SetInteger("gIOR", 2);
			SetInteger("gDepth", 3);
			SetInteger("gSSAOMap", SHADOWMAP_START_TEXTURE_UNIT + OMNI_SHADOW_MAP_COUNT * 2 + 1);
			FixSamplerProblem();
			break;
		case Graphics::EET_SSAO:
			SetInteger("gNormalRoughness", 0);
			SetInteger("gDepth", 1);
			SetInteger("texNoise", 2);
			break;
		case Graphics::EET_SSAO_Blur:
			SetInteger("ssaoInput", 0);
			break;
		case Graphics::EET_CubemapDisplayer:
			SetInteger("cubemapTex", 0);
			break;
		case Graphics::EET_SelectionBuffer:
			break;
		case Graphics::EET_Outline:
			SetFloat("outlineWidth", 0.15f);
			SetVec3("unlitColor", glm::vec3(Constants::g_outlineColor.r, Constants::g_outlineColor.g, Constants::g_outlineColor.b));
			break;
		case Graphics::EET_Billboards:
			SetInteger("sprite", 0);
			break;
		case Graphics::EET_Comp_Particle:
			break;
		case Graphics::EET_Invalid:
			break;
		default:
			break;
		}
		UnUseEffect();
		assert(GL_NO_ERROR == glGetError());
		return true;
	}

	bool cEffect::IsPathNull(const char* const i_incomingPath)
	{
		if ((i_incomingPath != nullptr) && (i_incomingPath[0] == '\0')) {
			return true;
		}
		return false;
	}

	bool cEffect::IsUniformIDValid(const GLuint& i_id)
	{
		return i_id >= 0 && i_id < static_cast<GLuint>(-1);
	}

	void cEffect::UseEffect()
	{
		// Bind program
		glUseProgram(m_programID);

	}

	void cEffect::UnUseEffect()
	{
		glUseProgram(0);
	}

	bool cEffect::RecompileShader(const char* i_shaderName, GLenum i_shaderType)
	{

		return true;
	}

	void cEffect::SetInteger(const char* const i_uniformName, const GLint& i_int)
	{
		GLuint _ID = glGetUniformLocation(m_programID, i_uniformName);
		if (IsUniformIDValid(_ID))
		{
			glUniform1i(_ID, i_int);
			assert(GL_NO_ERROR == glGetError());
		}
	}

	void cEffect::SetFloat(const char* const i_uniformName, const GLfloat& i_float)
	{
		GLuint _ID = glGetUniformLocation(m_programID, i_uniformName);
		if (IsUniformIDValid(_ID)) glUniform1f(_ID, i_float);

		assert(GL_NO_ERROR == glGetError());
	}

	void cEffect::SetVec3(const char* const i_uniformName, const glm::vec3& i_vec3)
	{
		GLuint _ID = glGetUniformLocation(m_programID, i_uniformName);
		if (IsUniformIDValid(_ID)) glUniform3f(_ID, i_vec3.x, i_vec3.y, i_vec3.z);

		assert(GL_NO_ERROR == glGetError());
	}

	bool cEffect::LoadShader(const char* i_shaderName, GLenum i_shaderType)
	{
		// this type of shader did not exist in GL context
		if (m_shaders.count(i_shaderType) <= 0) {
			sGLShader* _newShader = new sGLShader();
			if (!_newShader->CompileShader(i_shaderName, i_shaderType)) {
				printf("Error compiling shader[%s]\n", i_shaderName);
				return false;
			}
			m_shaders.insert({ i_shaderType, _newShader });
		}
		else {
			// if this shader already existed in this effect, need to recompile it
			if (!m_shaders.at(i_shaderType)->CompileShader(i_shaderName, i_shaderType)) {
				printf("Error compiling shader[%s]\n", i_shaderName);
				return false;
			}
		}

		// Attach this shader to this program
		glAttachShader(m_programID, m_shaders.at(i_shaderType)->GetShaderID());
		return true;
	}
	void cEffect::FixSamplerProblem()
	{
		// Fix sampler problem before validating the program
		char _charBuffer[64] = { '\0' };

		SetInteger("BrdfLUTMap", 4);
		SetInteger("AOMap", 5);

		const auto maxCubemapMixing = EnvironmentCaptureManager::MaximumCubemapMixingCount();
		constexpr auto cubemapStartID = IBL_CUBEMAP_START_TEXTURE_UNIT;
		for (size_t i = 0; i < maxCubemapMixing; ++i)
		{
			snprintf(_charBuffer, sizeof(_charBuffer), "IrradianceMap[%d]", i);
			SetInteger(_charBuffer, cubemapStartID + i);
			snprintf(_charBuffer, sizeof(_charBuffer), "PrefilterMap[%d]", i);
			SetInteger(_charBuffer, cubemapStartID + maxCubemapMixing + i);
		}
		for (int i = 0; i < OMNI_SHADOW_MAP_COUNT; ++i)
		{
			snprintf(_charBuffer, sizeof(_charBuffer), "spotlightShadowMap[%d]", i);
			SetInteger(_charBuffer, SHADOWMAP_START_TEXTURE_UNIT + i);
			snprintf(_charBuffer, sizeof(_charBuffer), "pointLightShadowMap[%d]", i);
			SetInteger(_charBuffer, SHADOWMAP_START_TEXTURE_UNIT + OMNI_SHADOW_MAP_COUNT + i);
		}
		SetInteger("directionalShadowMap", SHADOWMAP_START_TEXTURE_UNIT + OMNI_SHADOW_MAP_COUNT * 2);
		assert(GL_NO_ERROR == glGetError());

	}
}
