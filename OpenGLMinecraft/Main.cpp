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

glm::vec3 rotate(glm::vec3 pos, glm::vec2 origin, float increment);
glm::vec3 animate(glm::vec3 pos, glm::vec3 target, float increment);

//status variables for UI
bool clampMouse;
bool pastClampMouse;

//status ingame
bool rotating = false;
bool animating = false;
glm::vec3 animationTarget;
float animationTime = 20;
float travelDistance;

//control mouse sense and velocity
float speedMultiplier = 1;
float senseMultiplier = 1;

float speedIncrement = 1.1;
float senseIncrement = 1.1;

float sensitivity = 0.1f;

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

const double fps = 60;

//world settings
double treeFrequency = 0.001;
int mapSize = 300;
int mapAmplitude = 120;

Camera camera = Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(mapAmplitude / 2, mapAmplitude, -mapAmplitude/2));

GraphicsEngine *gE;

int main() {
	std::cout << "setup" << std::endl;

	//set priorety
	//SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);

	GraphicsEngine graphicsEngine("OpenGL Minecraft", &camera, &SCR_WIDTH, &SCR_HEIGHT);
	gE = &graphicsEngine;
	glfwSetCursorPosCallback(graphicsEngine.window, mouse_callback);
	glfwSetWindowFocusCallback(graphicsEngine.window, window_focus_callback);
	glfwSetMouseButtonCallback(graphicsEngine.window, mouse_button_callback);

	//status vars
	rotating = false;
	animating = false;
	animationTarget = graphicsEngine.light.pos;

	//graphicsEngine.minecraft.addBlockType(BlockType("Container", "resources/textures/GrassUnwrapped.jpg"));
	graphicsEngine.minecraft.addBlockType(BlockType("Grass", "resources/textures/GrassUnwrapped.jpg"));
	graphicsEngine.minecraft.addBlockType(BlockType("Dirt", "resources/textures/DirtUnwrapped.jpg"));
	graphicsEngine.minecraft.addBlockType(BlockType("Trunk", "resources/textures/TrunkUnwrapped.jpg"));
	graphicsEngine.minecraft.addBlockType(BlockType("Leaves", "resources/textures/LeavesUnwrapped.jpg"));
	//graphicsEngine.minecraft.addBlockType(BlockType("Light", "resources/textures/white.jpg"));
	//graphicsEngine.minecraft.addBlockType(BlockType("Ahegao", "resources/textures/ahegao.png"));

	std::vector<Block> blocks;

	//std::vector<std::vector<float>> heightMap = PerlinNoise::generate(100, 2, 40, 2, 5, 0.1);
	std::vector<std::vector<float>> heightMap = PerlinNoise::generate(mapSize, 2, mapAmplitude, 2, 5, 0.1);

	srand(time(0));
	for (int x = 0; x < mapSize; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = 0; z < mapSize; z++) {
				//std::cout << heightMap[x][z] << std::endl;
				if (y < 2) {
					blocks.push_back(Block(graphicsEngine.minecraft.blockType[1], 1, glm::vec3(x, y + int((heightMap[x][z])), z)));
				}
				else if (y > 0) {
					blocks.push_back(Block(graphicsEngine.minecraft.blockType[0], 0, glm::vec3(x, y + int((heightMap[x][z])), z)));
					if ((double)rand() / (RAND_MAX) <= treeFrequency) {
						makeTree(&blocks, glm::vec3(x, y + int((heightMap[x][z])), z));
					}
				}
			}
		}
	}

	//makeTree(&blocks, glm::vec3(5, 2 + int((heightMap[5][5])), 5));


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

	//set light pos
	//graphicsEngine.light.pos = glm::vec3(mapSize/2, mapAmplitude*1, mapSize/2);
	graphicsEngine.light.pos = glm::vec3(0, mapAmplitude, mapSize / 2);

	for (int i = 0; i < blocks.size(); i++) {
		if (blocks[i].pos.x >= 0 && blocks[i].pos.z >= 0)
			graphicsEngine.minecraft.addBlock(&blocks[i]);
	}

	//generate textures before render
	graphicsEngine.minecraft.compileVertices();
	graphicsEngine.minecraft.generateTextures();
	graphicsEngine.shadow.calculateShadows();

	//std::cout << graphicsEngine.loadedBlocks[0].size() << std::endl;

	//set cursor
	glfwSetInputMode(graphicsEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	clampMouse = true;

	int fpsCount = 0;
	int fpsCounter = 0;

	int gameState = 1;
	while (gameState == 1) {
		//start timer
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();


		//input
		processInput(graphicsEngine.window, &camera);

		//update game world info
		//update light so it moves
		if (rotating) {
			graphicsEngine.light.pos = rotate(graphicsEngine.light.pos, glm::vec2(mapSize / 2, mapSize / 2), 0.2);
		}

		if (animating) {
			glm::vec3 prevPos = graphicsEngine.light.pos;

			//make it move so that it reaches the target in 20 seconds and cancel when it gets close enough
			graphicsEngine.light.pos = animate(graphicsEngine.light.pos, animationTarget, travelDistance / (animationTime * fps));
			if (graphicsEngine.light.pos == prevPos) {
				animating = false;
			}
		}


		//render frame
		gameState = graphicsEngine.renderFrame();
		//graphicsEngine.shadow.calculateShadows();
		//glfwSwapBuffers(graphicsEngine.window);
		//glfwPollEvents();


		//end of timer sleep and normalize the clock
		std::chrono::system_clock::time_point after = std::chrono::system_clock::now();
		std::chrono::microseconds difference(std::chrono::time_point_cast<std::chrono::microseconds>(after) - std::chrono::time_point_cast<std::chrono::microseconds>(now));

		int diffCount = difference.count();
		if (diffCount == 0) {
			diffCount = 1;
		}

		int sleepDuration = ((1000000 / fps * 1000) - diffCount) / 1000000;

		//output fps
		fpsCount += 1;
		fpsCounter += 1000000 / diffCount;

		if (fpsCount % int(fps) == 0) {
			std::cout << "\rFPS: " << fpsCounter / fpsCount;
			fpsCount = 0;
			fpsCounter = 0;
		}

		if (sleepDuration < 0) {
			sleepDuration = 0;
		}

		//std::cout << sleepDuration << std::endl;
		Sleep(sleepDuration);
	}

	//end fps count segment
	std::cout << std::endl;

	//closing
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
		(*blocks).push_back(Block((*gE).minecraft.blockType[trunk], trunk, glm::vec3(pos.x, pos.y + y, pos.z)));
	}

	//first and second layer of leaves
	for (int y = 4; y <= 5; y++) {
		for (int x = -2; x <= 2; x++) {
			for (int z = -2; z <= 2; z++) {
				if (x != 0 || z != 0) {
					(*blocks).push_back(Block((*gE).minecraft.blockType[leaves], leaves, glm::vec3(pos.x + x, pos.y + y, pos.z + z)));
				}
			}
		}
	}

	//3rd and 4th layer
	for (int y = 6; y <= 7; y++) {
		for (int x = -1; x <= 1; x++) {
			for (int z = -1; z <= 1; z++) {
				if (((y == 6) || (y == 7 && abs(x) != abs(z))) && (x != 0 || z != 0)) {
					(*blocks).push_back(Block((*gE).minecraft.blockType[leaves], leaves, glm::vec3(pos.x + x, pos.y + y, pos.z + z)));
				}
			}
		}
	}

	//top the tree off
	(*blocks).push_back(Block((*gE).minecraft.blockType[leaves], leaves, glm::vec3(pos.x, pos.y + 7, pos.z)));
}

//circular movement function rotates parallel with ground and counter clockwise by increment in degrees
glm::vec3 rotate(glm::vec3 p, glm::vec2 origin, float increment) {
	glm::vec2 pos = glm::vec2(p.x, p.z) - origin;
	//get radius and account for the wierd positivity only glitch (it only went around half of the circle)
	float r = glm::distance(glm::vec2(p.x, p.z), origin) * (abs(pos.x) / pos.x);

	float angle = atan(pos.y / pos.x) * (180 / 3.1415926);
	angle += increment;

	//std::cout << "\r" << angle;

	return glm::vec3(origin.x + cos(angle / (180 / 3.1415926)) * r, p.y, origin.y + sin(angle / (180 / 3.1415926)) * r);
}

glm::vec3 animate(glm::vec3 pos, glm::vec3 target, float increment) {
	glm::vec3 dir = glm::normalize(target - pos);
	glm::vec3 newPos = dir * increment + pos;

	//if its close enough then stop otherwise move forward
	if (glm::distance(newPos, target) < increment) {
		return pos;
	}

	return newPos;
}

// process all input: ask GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window, Camera *camera)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		if (clampMouse) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			clampMouse = false;
		}
		else if (pastClampMouse == false) {
			glfwSetWindowShouldClose(window, true);
		}
	}
	else {
		pastClampMouse = clampMouse;
	}

	float accel = 0.02 * speedMultiplier;
	(*camera).deceleration = 0.01 * speedMultiplier;

	//speed controls
	//up
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		speedMultiplier *= speedIncrement;
	}
	//down
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		speedMultiplier /= speedIncrement;
	}

	//mouse controls
	//right
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		senseMultiplier *= senseIncrement;
	}

	//left
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		senseMultiplier /= senseIncrement;
	}

	//camera controls
	//W
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		(*camera).vel += accel * (*camera).Front;
	}
	//A
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		(*camera).vel -= accel * (*camera).Right;
	}
	//S
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		(*camera).vel -= accel * (*camera).Front;
	}
	//D
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		(*camera).vel += accel * (*camera).Right;
	}

	//SPACE
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		(*camera).vel += accel * glm::vec3(0.0f, 1.0f, 0.0f);
	}
	//CTRL
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		(*camera).vel -= accel * glm::vec3(0.0f, 1.0f, 0.0f);
	}

	//ENTER
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
		rotating = !rotating;
	}

	//P sets light pos
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		(*gE).light.pos = (*camera).pos;
	}

	//O sets the target for the light to move witht he animate function
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		animationTarget = (*camera).pos;
	}

	//RSHIFT starts the animation
	if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
		animating = true;
		travelDistance = glm::distance((*gE).light.pos, animationTarget);
	}

	(*camera).updateVelocity();
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

		//float sensitivity = 0.1f; // change this value to your liking
		xoffset *= sensitivity * senseMultiplier;
		yoffset *= sensitivity * senseMultiplier;

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