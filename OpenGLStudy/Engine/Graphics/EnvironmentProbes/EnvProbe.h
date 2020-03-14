/*
	EnvProbe stands for environment probes, is a technique of capturing real-world imagery from a camera and organizing that information into an environment texture,
	such as a cube map, that contains the view in all directions from a certain point in the scene.

	Use 6 cameras from different direction to capture the environment before the actual rendering
	1. For diffuse irradiance, it only capture once
	2. For specular irradiance, it needs multiple LODs depending on the roughness of the surface, normally 5.

	Other issues: the texture format of the cube map should be HDR textures since we need to take cube map as lights ource, so the low dynamic range texture does not work well.
*/

#pragma once

#include "Math/Transform/Transform.h"
#include "Graphics/FrameBuffer/cFrameBuffer.h"

namespace Graphics
{
	class cEnvProbe
	{
	public:
		cEnvProbe() {}

		~cEnvProbe() { CleanUp(); };

		bool Initialize(GLfloat i_range, GLuint i_width, GLuint i_height, const glm::vec3& i_initialLocation = glm::vec3(0));
		bool CleanUp();

		bool IsValid() const { return m_frameBuffer.IsValid() && m_range > 0 && m_width > 0 && m_height > 0 && m_width == m_height; }

		void StartCapture();
		void StopCapture();
		/** Getters */
		GLuint GetCubemapTextureID() const;
		glm::vec3 GetPosition() const { return m_transform.Position(); }
		Assets::cHandle<cTexture> GetCubemapTextureHandle() const { return m_frameBuffer.GetTextureHandle(); }
		glm::mat4 GetProjectionMat4() const {
			return glm::perspective(glm::radians(90.f), 1.f, 1.f, m_range);
		}
		glm::mat4 GetViewMat4(GLuint i_face) const;
		GLuint GetWidth() const { return m_width; }
		GLuint GetHeight() const { return m_height; }
	private:

		cTransform m_transform;
		cFrameBuffer m_frameBuffer;
		bool m_captured;

		// the maximum distance of this environment probe will capture
		GLfloat m_range;
		GLuint m_width, m_height; // resolution of the frame buffer
	};

}