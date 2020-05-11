#pragma once
#include "Graphics/Light/Light.h"

namespace Graphics {

	class cPointLight : public cGenLight
	{
	public:
		cPointLight() : m_range(0), cGenLight()
		{}
		cPointLight(Color i_color, const glm::vec3& i_position, GLfloat i_range);
		virtual ~cPointLight() { m_range = 0; }

		cPointLight(const cPointLight& i_other) = default;
		cPointLight& operator =(const cPointLight& i_other) = default;

		/** overriding virtual functions*/
		void Illuminate() override;
		void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0) override;
		void CreateShadowMap(GLuint i_width, GLuint i_height) override;
		void SetLightUniformTransform() override;
		void UseShadowMap(GLuint i_textureUnit) override;
		void SetShadowmapIdxAndResolutionIdx(GLuint i_shadowMapIdx, GLuint i_resolutionIDx) { m_shadowMapIdx = i_shadowMapIdx; m_resolutionIdx = i_resolutionIDx; };
		void CalculateDistToEye(const glm::vec3& i_eyePos);
		GLfloat Importance() const { return m_range / (0.01f + m_distToEye); }
		
		GLuint ShadowMapIdx() const { return m_shadowMapIdx; }
		GLuint ResolutionIdx() const { return m_resolutionIdx; }
		glm::mat4 GetViewMatrix() const;

		GLuint ImportanceOrder = 0;
	protected:
		// use inverse squared fall off
		GLfloat m_range = 100.f;
		GLuint m_lightTransformID = static_cast<GLuint>(-1), m_lightShadowMapID = static_cast<GLuint>(-1);
		GLuint m_shadowMapIdx = -1, m_resolutionIdx = 0;
		GLfloat m_distToEye = 1.f;
	private:
		GLuint m_farPlaneID = static_cast<GLuint>(-1);
	};

}