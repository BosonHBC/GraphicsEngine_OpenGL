#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Engine/Cores/BitArray/BitArray.h"

// Input struct
struct sWindowInput {

	sWindowInput(size_t i_numOfKeySupporting) {
		// Create a bit array with the size of supporting Key count, and initialize them all to 0
		m_keyDowns = Core::cBitArray::CreateBitArray(i_numOfKeySupporting, true);
	}
	~sWindowInput() {
		if (m_keyDowns) {
			delete m_keyDowns;
			m_keyDowns = nullptr;
		}
	}

	/** Manipulation functions */
	void SetKey(int i_key, bool i_down) {
		if(i_down){
			m_keyDowns->SetBit(i_key);
		}
		else {
			m_keyDowns->ClearBit(i_key);
		}
	}

	/** public variables */
	GLfloat lastX, lastY, dx, dy;
	bool isFirstMove =true;

	/** Query functions*/
	bool IsKeyDown(int i_key) { return m_keyDowns->IsBitSet(i_key); }
	bool IsKeyUp(int i_key) { return m_keyDowns->IsBitClear(i_key); }

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

private:
	Core::cBitArray* m_keyDowns;
};