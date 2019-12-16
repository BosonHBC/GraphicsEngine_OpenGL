#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"

// Input struct
struct sWindowInput {
	GLfloat lastX, lastY, dx, dy;
	uint64_t keyDowns = 0;
	bool isFirstMove = false;
	bool isMovingSinceLastChange = false;

	bool IsKeyDown(int i_key) {
		return (keyDowns >> i_key) & 1;
	}
	bool IsKeyUp(int i_key) {
		return !((keyDowns >> i_key) & 1);
	}

	GLfloat DX() {
		GLfloat tempDx = dx;
		dx = 0.0;
		return tempDx;
	}
	GLfloat DY() {
		GLfloat tempDy = dy;
		dy = 0.0;
		return tempDy;
	}
};