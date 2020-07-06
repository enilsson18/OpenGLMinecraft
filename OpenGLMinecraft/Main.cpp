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

//status variables for UI
bool clampMouse;
bool pastClampMouse;

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

const double fps = 60;

Camera camera = Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0, 1, -5));

int main() {
	std::cout << "setup" << std::endl;

	GraphicsEngine graphicsEngine("OpenGL Minecraft", &camera, SCR_WIDTH, SCR_HEIGHT);
	glfwSetCursorPosCallback(graphicsEngine.window, mouse_callback);
	glfwSetWindowFocusCallback(graphicsEngine.window, window_focus_callback);
	glfwSetMouseButtonCallback(graphicsEngine.window, mouse_button_callback);

	std::cout << "rendering" << std::endl;

	//graphicsEngine.addBlockType(BlockType("Container", "resources/textures/GrassUnwrapped.jpg"));
	graphicsEngine.addBlockType(BlockType("Container", "resources/textures/GrassUnwrapped.jpg"));
	
	std::vector<Block> blocks;

	std::vector<std::vector<float>> heightMap = PerlinNoise::generate(50);

	for (int x = 0; x < 50; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 50; z++) {
				blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(x, y + -int((heightMap[x][z])), z)));
			}
		}
	}
	

	
	blocks.push_back(Block(graphicsEngine.blockType[0], glm::vec3(0, 0, 0)));
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
		(*camera).pos.y += 0.1f;
	}
	//CTRL
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		(*camera).pos.y -= 0.1f;
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

		glm::vec3 front;
		front.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
		front.y = sin(glm::radians(camera.pitch));
		front.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
		camera.cameraFront = glm::normalize(front);
	}
}