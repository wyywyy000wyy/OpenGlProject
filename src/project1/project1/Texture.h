#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture {
public:
	Texture(const char* path);

	unsigned int m_textureID;
};