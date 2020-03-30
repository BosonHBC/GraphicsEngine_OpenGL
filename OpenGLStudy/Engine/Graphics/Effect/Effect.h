#pragma once

#include "GL/glew.h"
#include <map>
#include "glm/gtc/matrix_transform.hpp"
namespace Graphics {
	/** Effect type*/
	enum eEffectType : uint16_t
	{
		EET_BlinnPhong,
		EET_ShadowMap,
		EET_OmniShadowMap,
		EET_Cubemap,
		EET_Unlit,
		EET_NormalDisplay,
		EET_PBR_MR,
		EET_HDRToCubemap,
		EET_IrradConvolution,
		EET_CubemapPrefilter,
		EET_BrdfIntegration,
		EET_DrawDebugCircles,
		EET_TessQuad,
		EET_TriangulationDisplay,
		EET_HDREffect,
		EET_Invalid = static_cast<uint16_t>(-1),
	};
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
		bool CreateProgram(const char* const i_vertexShaderPath, const char* const i_fragmentShaderPath, const char* const i_geometryShaderPath = "", const char* const i_TCSPath = "", const char* const i_TESPath = "");
		bool LinkProgram();
		bool ValidateProgram();
		void CleanUp();

		/** Usage functions*/
		void UseEffect();
		void UnUseEffect();
		bool RecompileShader(const char* i_shaderName, GLenum i_shaderType);
		void SetInteger(const char* const i_uniformName, const GLint& i_int);
		void SetFloat(const char* const i_uniformName, const GLfloat& i_float);
		void SetVec3(const char* const i_uniformName, const glm::vec3& i_vec3);
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

		bool IsPathNull(const char* const i_incomingPath);

		bool IsUniformIDValid(const GLuint& i_id);
	};

}
