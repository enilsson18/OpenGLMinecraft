#ifndef BLOCK_H
#define BLOCK_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include "BlockType.h"

#include <string>
#include <iostream>

class Block {
public:
	BlockType blockType;
	glm::vec3 pos;
	int id;

	Block(BlockType blockInfo, glm::vec3 pos) {
		this->blockType = blockInfo;
		this->pos = pos;
	}

	Block(BlockType blockInfo, int id, glm::vec3 pos) {
		this->blockType = blockInfo;
		this->pos = pos;
		this->id = id;
	}
};
#endif