#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Engine/Cores/BitArray/BitArray.h"

// Input struct
struct sWindowInput {

	sWindowInput(size_t i_numOfKeySupporting) : lastX(0), lastY(0), dx(0), dy(0)
	{
		// Create a bit array with the size of supporting Key count, and initialize them all to 0
		m_keyDowns = Core::cBitArray::CreateBitArray(i_numOfKeySupporting, true);
		// for mouse button, it only has 8 situations
		m_buttonDowns = Core::cBitArray::CreateBitArray(8, true);
	}
	~sWindowInput() {
		if (m_keyDowns) {
			delete m_keyDowns;
			m_keyDowns = nullptr;
		}
		if (m_buttonDowns) {
			delete m_buttonDowns;
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
	void SetButton(int i_button, bool i_down) {
		if (i_down) {
			m_buttonDowns->SetBit(i_button);
		}
		else {
			m_buttonDowns->ClearBit(i_button);
		}
	}

	/** public variables */
	GLfloat lastX, lastY, dx, dy;
	bool isFirstMove =true;

	/** Query functions*/
	bool IsKeyDown(int i_key) { return m_keyDowns->IsBitSet(i_key); }
	bool IsKeyUp(int i_key) { return m_keyDowns->IsBitClear(i_key); }
	bool IsButtonDown(int i_button) { return m_buttonDowns->IsBitSet(i_button); }
	bool IsButtonUp(int i_button) { return m_buttonDowns->IsBitClear(i_button); }

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
	Core::cBitArray* m_buttonDowns;
};