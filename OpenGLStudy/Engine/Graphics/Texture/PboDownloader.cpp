#include "PboDownloader.h"
#include <stdio.h>
#include "assert.h"
#include <cstring>
// referencing pages: https://www.roxlu.com/2014/048/fast-pixel-transfers-with-pixel-buffer-objects
namespace Graphics {

	PboDownloader::PboDownloader()
		:fmt(0)
		, pbos(nullptr)
		, num_pbos(0)
		, dx(0)
		, num_downloads(0)
		, width(0)
		, height(0)
		, nbytes(0)
		, pixels(nullptr)
	{
	}

	int PboDownloader::init(GLenum format, int w, int h, int num) {

		if (nullptr != pbos) {
			printf("Already initialized. Not necessary to initialize again; or shutdown first.");
			return -1;
		}

		if (0 >= num) {
			printf("Invalid number of PBOs: %d", num);
			return -2;
		}

		if (num > 10) {
			printf("Asked to create more then 10 buffers; that is probaly a bit too much.");
		}

		fmt = format;
		width = w;
		height = h;
		num_pbos = num;

		if (GL_RED == fmt || GL_GREEN == fmt || GL_BLUE == fmt) {
			nbytes = width * height;
		}
		else if (GL_RGB == fmt || GL_BGR == fmt) {
			nbytes = width * height * 3;
		}
		else if (GL_RGBA == fmt || GL_BGRA == fmt) {
			nbytes = width * height * 4;
		}
		else {
			printf("Unhandled pixel format, use GL_R, GL_RG, GL_RGB or GL_RGBA.");
			return -3;
		}

		if (0 == nbytes) {
			printf("Invalid width or height given: %d x %d", width, height);
			return -4;
		}

		pbos = new GLuint[num];
		if (nullptr == pbos) {
			printf("Cannot allocate pbos.");
			return -3;
		}

		pixels = new unsigned char[nbytes];
		if (nullptr == pixels) {
			printf("Cannot allocate pixel buffer.");
			return -5;
		}

		glGenBuffers(num, pbos);
		for (int i = 0; i < num; ++i) {

			printf("pbodownloader.pbos[%d] = %d, nbytes: %d\n", i, pbos[i], nbytes);

			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[i]);
			glBufferData(GL_PIXEL_PACK_BUFFER, nbytes, nullptr, GL_STREAM_READ);
		}

		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		return 0;
	}

	void PboDownloader::download() {
		unsigned char* ptr;
		if (num_downloads < num_pbos) {
			/*
			   First we need to make sure all our pbos are bound, so glMap/Unmap will
			   read from the oldest bound buffer first.
			*/
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[dx]);
			glReadPixels(0, 0, width, height, fmt, GL_UNSIGNED_BYTE, 0);   /* When a GL_PIXEL_PACK_BUFFER is bound, the last 0 is used as offset into the buffer to read into. */
			//printf("glReadPixels() with pbo: %d", pbos[dx]);
		}
		else {
			/* Read from the oldest bound pbo. */
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[dx]);

			ptr = (unsigned char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
			if (nullptr != ptr) {
				memcpy(pixels, ptr, nbytes);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			}
			else {
				assert(false);
			}

			/* Trigger the next read. */
			glReadPixels(0, 0, width, height, fmt, GL_UNSIGNED_BYTE, 0);
		}

		++dx;
		dx = dx % num_pbos;

		num_downloads++;
		if (num_downloads == UINT64_MAX) {
			num_downloads = num_pbos;
		}
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}

	void PboDownloader::cleanUp()
	{
		if (nullptr != pixels) {
			delete[] pixels;
			pixels = nullptr;
		}
	}

	unsigned char* PboDownloader::getPixel_rgb(int iw, int ih)
	{
		assert(iw < width && ih < height);
		return &pixels[(ih * width + iw) * 3];
	}

}