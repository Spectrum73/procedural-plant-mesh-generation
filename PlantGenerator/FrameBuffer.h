#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <stdio.h>
#include <iostream>
#include <glad/glad.h>
#include "imgui.h"

class FBO {
private:
	unsigned int ID;
	GLuint RBO;
	GLuint frameBufferTexture;
public:
	FBO();
	FBO(int aWidth, int aHeight);
	~FBO();
	void RescaleFrameBuffer(float aWidth, float aHeight);
	void Bind();
	void Unbind();
	GLuint GetTextureID();
	ImVec2 size;
};
#endif