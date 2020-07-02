#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <img/stb_image.h>
//#include <img/ImageLoader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include "GraphicsEngine.h"
#include "BlockType.h"
#include "Block.h"

#include <FileSystem>
#include <iostream>

#include <vector>
#include <chrono>
#include <cmath>
#include <windows.h>

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

const double fps = 60;

int main() {
	std::cout << "setup" << std::endl;
	GraphicsEngine graphicsEngine("OpenGL Minecraft", SCR_WIDTH, SCR_HEIGHT);
	std::cout << "rendering" << std::endl;

	graphicsEngine.addBlockType(BlockType("Container", "resources/textures/container.jpg"));
	
	std::vector<Block> blocks;

	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 10; y++) {
			for (int z = 0; z < 40; z++) {
				blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(-x, -y, -z)));
			}
		}
	}

	/*
	blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(0, 0, 0)));
	blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(1, 0, 0)));
	blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(-1, 0, 0)));
	blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(1, -1, 0)));
	blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(-1, -1, 0)));
	blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(0, -2, 0)));
	blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(-1, -2, 0)));
	blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(1, -2, 0)));
	*/

	for (int i = 0; i < blocks.size(); i++) {
		graphicsEngine.addBlock(&blocks[i]);
	}

	//generate textures before render
	graphicsEngine.generateTextures();

	//std::cout << graphicsEngine.surfaces[0].size() << std::endl;


	int gameState = 1;
	while (gameState == 1) {
		//start timer
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();


		//input
		processInput(graphicsEngine.window,&graphicsEngine.camera);

		//update game info and send data to renderer


		//render frame
		gameState = graphicsEngine.renderFrame();


		//end of timer sleep and normalize the clock
		std::chrono::system_clock::time_point after = std::chrono::system_clock::now();
		std::chrono::milliseconds difference(std::chrono::time_point_cast<std::chrono::milliseconds>(after) - std::chrono::time_point_cast<std::chrono::milliseconds>(now));

		int sleepDuration = (1000 / fps) - difference.count();
		std::cout << sleepDuration << std::endl;
		if (sleepDuration < 0) {
			sleepDuration = 0;
		}

		Sleep(sleepDuration);
	}
	
	std::cout << "ending" << std::endl;
	graphicsEngine.terminate();
	return 0;
}

// process all input: ask GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window, Camera *camera)
{
	//escape key
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	//camera controls
	//W
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		(*camera).pos.z += 0.1f;
	}
	//A
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		(*camera).pos.x += 0.1f;
	}
	//S
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		(*camera).pos.z -= 0.1f;
	}
	//D
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		(*camera).pos.x -= 0.1f;
	}

	//SPACE
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		(*camera).pos.y -= 0.1f;
	}
	//CTRL
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		(*camera).pos.y += 0.1f;
	}
}