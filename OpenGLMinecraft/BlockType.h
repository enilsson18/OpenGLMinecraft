#ifndef BLOCKTYPE_H
#define BLOCKTYPE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include <vector>
#include <string>
#include <iostream>

class BlockType {
public:
	std::string name;
	const char* texture;

	BlockType() {
		
	}

	BlockType(const std::string name, const char* texturePath) {
		this->name = name;
		texture = texturePath;
	}
};

#endif
