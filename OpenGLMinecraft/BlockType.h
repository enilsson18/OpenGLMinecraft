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
	std::vector<const char*> texture;

	BlockType() {
		
	}

	BlockType(const std::string name, const char* texturePath) {
		this->name = name;
		for (int i = 0; i < 6; i++) {
			texture.push_back(texturePath);
		}
	}

	BlockType(const std::string name, std::vector<const char*> textures) {
		this->name = name;
		if (textures.size() == 6) {
			texture = textures;
		}
		else {
			texture.push_back(textures[0]);
		}
	}
};

#endif
