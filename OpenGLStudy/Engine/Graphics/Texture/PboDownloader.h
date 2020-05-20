#pragma once
#include "GL/glew.h"
#include "glfw/glfw3.h"

namespace Graphics {

	class PboDownloader {
	public:
		PboDownloader();
		~PboDownloader() {};
		int init(GLenum fmt, int w, int h, int num);
		void download();
		void cleanUp();
		unsigned char* getPixel_rgb(int iw, int ih);
	public:
		GLenum fmt;
		GLuint* pbos;
		uint64_t num_pbos;
		uint64_t dx;
		uint64_t num_downloads;
		int width;
		int height;
		int nbytes; /* number of bytes in the pbo buffer. */
		unsigned char* pixels; /* the downloaded pixels. */
	};

} 

#endif