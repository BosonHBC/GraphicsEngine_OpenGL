#pragma once
#include "GL/glew.h"
#include "glm/glm.hpp"

#include "Engine/Graphics/Color/Color.h"
#include "Cores/Core.h"
#include "Engine/Math/Transform/Transform.h"

namespace Graphics {
	// forward declaration
	class cFrameBuffer;

	// An Interface for all kinds of lights
	class cGenLight
	{
	public:
		/** Constructors and destructor */
		cGenLight();
		cGenLight(Color i_color);
		cGenLight(const cGenLight& i_other);
		cGenLight& operator = (const cGenLight& i_other);
		virtual ~cGenLight() { CleanUp(); }

		/**Usage function*/
		virtual void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0);
		void UpdateLightIndex(GLuint i_lightIndex);
		virtual void CleanUp();
		/** Pure virtual functions*/
		virtual void Illuminate() {};
		void SetColor(const Color& i_c) { m_color = i_c; }
		const Color& GetColor() const { return m_color; }
		/** Getters */

		/** Shadow map related*/
		void SetEnableShadow(bool i_enable) { m_enableShadow = i_enable; }
		virtual void CreateShadowMap(GLuint i_width, GLuint i_height);
		glm::mat4 GetProjectionmatrix() const { return m_lightPrjectionMatrix; }
		virtual glm::mat4 GetViewMatrix() const { return glm::mat4(1.0f); }
		virtual void SetLightUniformTransform() {};
		virtual void UseShadowMap(GLuint i_textureUnit) {}
		cFrameBuffer* GetShadowMap() const { return m_shadowMap; }
		bool IsShadowMapValid() const;
		bool IsShadowEnabled() const { return m_enableShadow && IsShadowMapValid(); }
		void CleanUpShadowMap();

		cTransform Transform;
	protected:
		Color m_color = Color(0,0,0);
		// record the index of this light
		GLuint m_lightIndex = 0;
		bool m_enableShadow = false;

		// this determine which kind of projection the shadow map wants
		// it should be type variant
		glm::mat4 m_lightPrjectionMatrix = glm::mat4(1.0f);
		cFrameBuffer* m_shadowMap = nullptr;
	};

}
