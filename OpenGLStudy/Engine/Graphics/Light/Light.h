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
		virtual ~cGenLight();

		/**Usage function*/
		virtual void SetupLight(const GLuint& i_programID, GLuint i_lightIndex = 0);
		cTransform* Transform() const { return m_transform; }

		/** Pure virtual functions*/
		virtual void Illuminate() = 0;

		/** Getters */


		/** Shadow map related*/
		virtual void CreateShadowMap(GLuint i_width, GLuint i_height);
		virtual glm::mat4 CalculateLightTransform() const { return glm::identity < glm::mat4 >(); };
		virtual void SetLightUniformTransform() {};
		cFrameBuffer* GetShadowMap() const { return m_shadowMap; }
		bool IsShadowMapValid() const { return m_shadowMap != nullptr; }
	protected:
		Color m_color;
		GLuint m_colorID;
		// record the index of this light
		GLuint m_lightIndex;

		cTransform* m_transform;
		// this determine which kind of projection the shadow map wants
		// it should be type variant
		glm::mat4 m_lightPrjectionMatrix;
		cFrameBuffer* m_shadowMap;
	};

}
