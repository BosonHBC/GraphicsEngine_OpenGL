/*
	EnvProbe stands for environment probes, is a technique of capturing real-world imagery from a camera and organizing that information into an environment texture,
	such as a cube map, that contains the view in all directions from a certain point in the scene.

	For this class, it stores a HDR cube map.

	Use 6 cameras from different direction to capture the environment before the actual rendering
	1. It can be used to capture a very detail version of the environment, by rendering the whole scene(actual geometries) 6 times.
	2. It can be used to generate irradiance map from any cube map, using very low resolution, usually 32 *32
	3. It can be used to generate specular pre-filtering maps, it needs multiple LODs depending on the roughness of the surface, normally 5.

	Other issues: the texture format of the cube map should be HDR textures since we need to take cube map as light source, so the low dynamic range texture does not work well.
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

		~cEnvProbe() { };
		cEnvProbe(const cEnvProbe& i_other) = default;
		cEnvProbe& operator = (const cEnvProbe& i_other) = default;

		bool Initialize(GLfloat i_range, GLuint i_width, GLuint i_height, const ETextureType& i_textureType, const glm::vec3& i_initialLocation = glm::vec3(0));
		bool CleanUp();

		bool IsValid() const { return m_frameBuffer->IsValid() && m_range > 0 && m_width > 0 && m_height > 0 && m_width == m_height && m_captured; }

		void StartCapture(const std::function<void()>& captureFunction);
		void StopCapture();

		/** Getters */
		GLuint GetCubemapTextureID() const;
		glm::vec3 GetPosition() const { return m_position; }
		Assets::cHandle<cTexture> GetCubemapTextureHandle() const { return m_frameBuffer->GetTextureHandle(); }
		glm::mat4 GetProjectionMat4() const {
			return glm::perspective(glm::radians(90.f), 1.f, 1.f, 2000.f);
		}
		glm::mat4 GetViewMat4(GLuint i_face) const;
		GLuint GetWidth() const { return m_width; }
		GLuint GetHeight() const { return m_height; }
		GLfloat GetRange() const { return m_range; }

		GLuint fbo() const { return m_frameBuffer->rbo(); }
		GLuint rbo() const { return m_frameBuffer->rbo(); }
	private:

		glm::vec3 m_position;
		cFrameBuffer* m_frameBuffer;
		bool m_captured;

		// the maximum distance of this environment probe will capture
		GLfloat m_range;
		GLuint m_width, m_height; // resolution of the frame buffer
	};

}