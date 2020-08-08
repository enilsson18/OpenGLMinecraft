#ifndef GRAPHICSENGINE_H
#define GRAPHICSENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <img/stb_image.h>
//idk but manually importing fixes the problem
//#include <img/ImageLoader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <shader.h>

#include "Camera.h"

#include "MinecraftManager.h"
#include "BlockType.h"
#include "Block.h"
#include "Surface.h"
#include "Light.h"
#include "Skybox.h"
#include "Shadow.h"
#include "Quad.h"

#include <vector>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

//callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Camera *camera);

class GraphicsEngine {

public:

	GLFWwindow* window;

	const unsigned int *SCR_WIDTH;
	const unsigned int *SCR_HEIGHT;

	//minecraft
	MinecraftManager minecraft;

	//shadows
	Shadow shadow = Shadow();

	//skybox
	Skybox skyBox;

	//camera
	Camera *camera;

	Light light;

	//debug quad
	Shader debug;
	Quad quad;

	GraphicsEngine(const char* windowName, Camera *cam, const unsigned int *scr_WIDTH, const unsigned int *scr_HEIGHT) {
		//minecraft
		minecraft = MinecraftManager();

		//initialize Camera
		camera = cam;

		//light
		light = Light(glm::vec3(25, 25, 25), glm::vec3(1, 1, 1));

		//debug quad
		quad = Quad();

		////shadows
		//shadow = Shadow(1);

		// glfw: initialize and configure
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		SCR_WIDTH = scr_WIDTH;
		SCR_HEIGHT = scr_HEIGHT;

		// glfw window creation
		window = glfwCreateWindow(*scr_WIDTH, *scr_HEIGHT, windowName, NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}
		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

		// glad: load all OpenGL function pointers
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
		}

		// configure global opengl state
		glEnable(GL_DEPTH_TEST);
		stbi_set_flip_vertically_on_load(true);

		//setup classes
		//minecraft
		minecraft.setup("resources/shaders/6.3.coordinate_systems.vs", "resources/shaders/6.3.coordinate_systems.fs");

		//light
		light.setup("resources/shaders/light_cube.vs", "resources/shaders/light_cube.fs");

		//skybox
		std::vector<const char*> skyBoxFaces
		{
			"resources/textures/CloudySkyBox/cubemap_1.jpg",
			"resources/textures/CloudySkyBox/cubemap_3.jpg",
			"resources/textures/CloudySkyBox/cubemap_4.jpg",
			"resources/textures/CloudySkyBox/cubemap_5.jpg",
			"resources/textures/CloudySkyBox/cubemap_0.jpg",
			"resources/textures/CloudySkyBox/cubemap_2.jpg",
		};

		skyBox.setup("resources/shaders/sky_box.vs", "resources/shaders/sky_box.fs", skyBoxFaces);

		//shadows
		shadow.setup("resources/shaders/shadow_depth.vs", "resources/shaders/shadow_depth.fs", &minecraft, camera, &light);

		//debug
		debug = Shader("resources/shaders/debug_quad.vs", "resources/shaders/debug_quad.fs");
		debug.use();
		debug.setInt("tex", 0);
	}

	int renderFrame() {
		if (glfwWindowShouldClose(window)) {
			return 0;
		}

		shadow.calculateShadows();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, *SCR_WIDTH, *SCR_HEIGHT);

		//handle input
		//processInput(window, &camera);

		//render
		//clear the screen and start next frame
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//draw skybox
		skyBox.render((*camera).projection, (*camera).update());

		//draw light source
		light.render((*camera).projection, (*camera).update());

		//draw world
		minecraft.renderWorld((*camera).projection, (*camera).update(), *camera, light, shadow.projectionType, shadow.depthMap, shadow.shadowMap, shadow.lightSpaceMatrix);

		//debug quad
		//debug.use();

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, shadow.shadowMap);

		//quad.render();

		//swap the buffers for smooth frames and poll events
		//std::cout << "events" << std::endl;
		glfwSwapBuffers(window);
		glfwPollEvents();

		return 1;
	}

	//end opengl and free allocated resources
	void terminate() {
		glfwTerminate();
	}
};

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	std::cout << "Failed to create GLFW window" << std::endl;
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on some displays
	glViewport(0, 0, width, height);
}

#endif