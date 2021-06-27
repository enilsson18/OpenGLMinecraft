#ifndef SURFACE_H
#define SURFACE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include "Block.h"
#include "Block.h"

#include <vector>
#include <string>
#include <iostream>

class Surface {
public:
	unsigned int orientation;
	BlockType blockType;
	Block* block;
	unsigned int* texture;

	//default
	Surface() {

	}

	Surface(Block *block, unsigned int *texture, BlockType blockType, int orientation) {
		(this->block) = block;
		this->texture = texture;
		this->blockType = blockType;
	}
};

#endif
