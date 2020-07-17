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
#include "PerlinNoise.h"

#include <FileSystem>
#include <iostream>

#include <vector>
#include <chrono>
#include <cmath>
#include <ctime>
#include <windows.h>

//prototypes
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void window_focus_callback(GLFWwindow* window, int focused);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void makeTree(std::vector<Block> *blocks, glm::vec3 pos);

//status variables for UI
bool clampMouse;
bool pastClampMouse;

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

const double fps = 60;

Camera camera = Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0, 30, -5));

//world settings
double treeFrequency = 0.001;

GraphicsEngine *gE;

int main() {
	std::cout << "setup" << std::endl;

	GraphicsEngine graphicsEngine("OpenGL Minecraft", &camera, SCR_WIDTH, SCR_HEIGHT);
	gE = &graphicsEngine;
	glfwSetCursorPosCallback(graphicsEngine.window, mouse_callback);
	glfwSetWindowFocusCallback(graphicsEngine.window, window_focus_callback);
	glfwSetMouseButtonCallback(graphicsEngine.window, mouse_button_callback);

	std::cout << "rendering" << std::endl;

	//graphicsEngine.addBlockType(BlockType("Container", "resources/textures/GrassUnwrapped.jpg"));
	graphicsEngine.addBlockType(BlockType("Grass", "resources/textures/GrassUnwrapped.jpg"));
	graphicsEngine.addBlockType(BlockType("Dirt", "resources/textures/DirtUnwrapped.jpg"));
	graphicsEngine.addBlockType(BlockType("Trunk", "resources/textures/TrunkUnwrapped.jpg"));
	graphicsEngine.addBlockType(BlockType("Leaves", "resources/textures/LeavesUnwrapped.jpg"));
	//graphicsEngine.addBlockType(BlockType("Ahegao", "resources/textures/ahegao.png"));
	
	std::vector<Block> blocks;

	//std::vector<std::vector<float>> heightMap = PerlinNoise::generate(100, 2, 40, 2, 5, 0.1);
	std::vector<std::vector<float>> heightMap = PerlinNoise::generate(100, 2, 40, 2, 5, 0.1);

	srand(time(0));
	for (int x = 0; x < 100; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = 0; z < 100; z++) {
				//std::cout << heightMap[x][z] << std::endl;
				if (y < 2) {
					blocks.push_back(Block(graphicsEngine.blockType[1], 1, glm::vec3(x, y + int((heightMap[x][z])), z)));
				}
				else if (y > 0) {
					blocks.push_back(Block(graphicsEngine.blockType[0], 0, glm::vec3(x, y + int((heightMap[x][z])), z)));
					if ((double)rand() / (RAND_MAX) <= treeFrequency) {
						makeTree(&blocks, glm::vec3(x, y + int((heightMap[x][z])), z));
					}
				}
			}
		}
	}
	

	
	//blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(0, 0, 0)));
	//blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(1, 1, 1)));
	/*blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(1, 0, 0)));
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
	graphicsEngine.compileVertices();
	graphicsEngine.generateTextures();

	//std::cout << graphicsEngine.loadedBlocks[0].size() << std::endl;

	//set cursor
	glfwSetInputMode(graphicsEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	clampMouse = true;

	int gameState = 1;
	while (gameState == 1) {
		//start timer
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();


		//input
		processInput(graphicsEngine.window,&camera);

		//update game info and send data to renderer


		//render frame
		gameState = graphicsEngine.renderFrame();


		//end of timer sleep and normalize the clock
		std::chrono::system_clock::time_point after = std::chrono::system_clock::now();
		std::chrono::microseconds difference(std::chrono::time_point_cast<std::chrono::microseconds>(after) - std::chrono::time_point_cast<std::chrono::microseconds>(now));

		int diffCount = difference.count();
		if (diffCount == 0) {
			diffCount = 1;
		}

		int sleepDuration = ((1000000 / fps * 1000) - diffCount)/1000000;
		std::cout << "FPS: " << 1000000/diffCount << std::endl;
		if (sleepDuration < 0) {
			sleepDuration = 0;
		}

		//std::cout << sleepDuration << std::endl;
		Sleep(sleepDuration);
	}
	
	std::cout << "ending" << std::endl;
	graphicsEngine.terminate();
	return 0;
}

//make trees 1 block ontop of the pos given
void makeTree(std::vector<Block> *blocks, glm::vec3 pos) {
	//texture positions
	int trunk = 2;
	int leaves = 3;

	for (int y = 1; y <= 6; y++) {
		//trunk
		(*blocks).push_back(Block((*gE).blockType[trunk], trunk, glm::vec3(pos.x, pos.y + y, pos.z)));
	}

	//first and second layer of leaves
	for (int y = 4; y <= 5; y++) {
		for (int x = -2; x <= 2; x++) {
			for (int z = -2; z <= 2; z++) {
				if (x != 0 || z != 0) {
					(*blocks).push_back(Block((*gE).blockType[leaves], leaves, glm::vec3(pos.x + x, pos.y + y, pos.z + z)));
				}
			}
		}
	}

	//3rd and 4th layer
	for (int y = 6; y <= 7; y++) {
		for (int x = -1; x <= 1; x++) {
			for (int z = -1; z <= 1; z++) {
				if (((y == 6) || (y == 7 && abs(x) != abs(z))) && (x != 0 || z != 0)) {
					(*blocks).push_back(Block((*gE).blockType[leaves], leaves, glm::vec3(pos.x + x, pos.y + y, pos.z + z)));
				}
			}
		}
	}

	//top the tree off
	(*blocks).push_back(Block((*gE).blockType[leaves], leaves, glm::vec3(pos.x, pos.y + 7, pos.z)));
}

// process all input: ask GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window, Camera *camera)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		if (clampMouse) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			clampMouse = false;
		}
		else if (pastClampMouse == false){
			glfwSetWindowShouldClose(window, true);
		}
	}
	else {
		pastClampMouse = clampMouse;
	}

	float velocity = 1.0f;

	//camera controls
	//W
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		(*camera).pos += velocity * (*camera).Front;
	}
	//A
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		(*camera).pos -= velocity * (*camera).Right;
	}
	//S
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		(*camera).pos -= velocity * (*camera).Front;
	}
	//D
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		(*camera).pos += velocity * (*camera).Right;
	}

	//SPACE
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		(*camera).pos += velocity * glm::vec3(0.0f, 1.0f, 0.0f);
	}
	//CTRL
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		(*camera).pos -= velocity * glm::vec3(0.0f, 1.0f, 0.0f);
	}
}

//focus callback
void window_focus_callback(GLFWwindow* window, int focused) {
	if (focused)
		clampMouse = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		clampMouse = true;
	}
		
}

//mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (clampMouse) {
		if (camera.firstMouse)
		{
			camera.lastX = xpos;
			camera.lastY = ypos;
			camera.firstMouse = false;
		}

		float xoffset = xpos - camera.lastX;
		float yoffset = camera.lastY - ypos; // reversed since y-coordinates go from bottom to top
		camera.lastX = xpos;
		camera.lastY = ypos;

		float sensitivity = 0.1f; // change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		camera.yaw += xoffset;
		camera.pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (camera.pitch > 89.0f)
			camera.pitch = 89.0f;
		if (camera.pitch < -89.0f)
			camera.pitch = -89.0f;

		//update vectors
		camera.updateCameraVectors();
	}
}