/*
	Geometry buffer
*/
#pragma once
#include "cFrameBuffer.h"

namespace Graphics
{
	class cGBuffer : public cFrameBuffer
	{
	public:
		cGBuffer() : cFrameBuffer() {}
		~cGBuffer() {};
		cGBuffer(const cGBuffer& i_other) :
			cFrameBuffer(i_other), m_normalHolder(i_other.m_normalHolder)
		{}
		bool Initialize(GLuint i_width, GLuint i_height);
		void CleanUp();

		void Read(GLenum* i_textureIDs);
		void ReadAlbedoMetallic(GLenum i_textureID);
		void ReadNormalRoughness(GLenum i_textureID);
		void ReadIOR(GLenum i_textureID);
		void ReadDepth(GLenum i_textureID);
	private:
		// m_renderToTexture will be responsible for ColorComponent0, recording albedo color in RGB channel, metallic in A channel
		// So the format Texture mode should be A8R8G8B8
		//	cTexture::HANDLE m_renderToTexture;

		// normal holder will be responsible for ColorComponent1, storing normal and smoothness
		// A16R16G16B16 
		cTexture::HANDLE m_normalHolder;

		// ior holder will be responsible for ColorComponent2, storing ior data
		// R16G16B16 
		cTexture::HANDLE m_iorHolder;

		// depth texture from the frame buffer
		cTexture::HANDLE m_depthHolder;
	};
}