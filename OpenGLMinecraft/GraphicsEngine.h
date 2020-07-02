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

#include <shader.h>

#include "Camera.h"

#include "BlockType.h"
#include "Block.h"
#include "Surface.h"

#include <vector>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Camera *camera);

class GraphicsEngine {

public:
	GLFWwindow* window;

	//shader
	Shader ourShader;

	//cube vertex data
	std::vector<unsigned int> VBO, VAO;

	//surfaces to render in standard order of orientation 6 x num of blocks
	std::vector<std::vector<Surface>> surfaces;

	//camera
	Camera camera;

	//block data
	std::vector<BlockType> blockType;
	//this vector is effectivly a way to divide the blocks based on block type for drawing. They are sorted before the main loop runs and requires less comparasons for assigning textures.
	std::vector<std::vector<Block*>> loadedBlocks;

	//texture data
	std::vector<std::vector<unsigned int>> texture;

	GraphicsEngine(const char* windowName, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT) {
		//initialize Camera
		camera = Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0,-1,-5));

		// glfw: initialize and configure
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// glfw window creation
		window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, windowName, NULL, NULL);
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

		// build and compile our shader zprogram
		ourShader = Shader("shaders/6.3.coordinate_systems.vs", "shaders/6.3.coordinate_systems.fs");

		// set up vertex data (and buffer(s)) and configure vertex attributes for blocks
		float faceVertices[6][30] = {
			{
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f
			},
			{
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f
			},
			{
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f
			},
			{
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f
			},
			{
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f
			},
			{
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
			}
		};

		//load and set pos of all the vertecies for construction of a cube
		VBO.resize(6);
		VAO.resize(6);
		for (int i = 0; i < 6; i++) {
			glGenVertexArrays(1, &VAO[i]);
			glGenBuffers(1, &VBO[i]);

			glBindVertexArray(VAO[i]);

			glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(faceVertices[i]), faceVertices[i], GL_STATIC_DRAW);

			// position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			// texture coord attribute
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
		}

		//test crate
		//addBlockType(BlockType("Crate", "resources/textures/container.jpg"));
		/*
		for (int x = 0; x < 10; x++) {
			for (int y = 0; y < 10; y++) {
				for (int z = 0; z < 40; z++) {
					addBlock(Block(blockType[0], glm::vec3(x, -y, z)));
				}
			}
		}
		*/
		//addBlock(&Block(blockType[0], glm::vec3(0, 0, 0)));

		//generateTextures();
	}

	//add block types and a new column to insert block data
	void addBlockType(BlockType blockType) {
		this->blockType.push_back(blockType);

		loadedBlocks.push_back(std::vector<Block*>());
	}

	//textures are in order of the block types as they were added to the manager
	void generateTextures() {
		texture.clear();
		for (int i = 0; i < blockType.size(); i++) {
			//for (int j = 0; j < 6; j++)
			texture.push_back(std::vector<unsigned int>());
			for (int t = 0; t < 6; t++) {
				texture[i].push_back(NULL);
				loadTexture(&texture[i][t], blockType[i].texture[t]);
			}
		}

		ourShader.use();
		ourShader.setInt("texture", 0);
	}

	void addBlock(Block *block) {
		//block type and loaded blocks 1st dimension should always be the same size because they are both only changed in the add block type method
		for (int i = 0; i < blockType.size(); i++) {
			if (blockType[i].name == (*block).blockType.name) {
				loadedBlocks[i].push_back(block);
			}
		}
	}

	bool loadTexture(unsigned int *texture, const char* fileName) {
		//load attributes and specify the read data of the block texture positions for rendering
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
		unsigned char *data = stbi_load(fileName, &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
			return false;
		}
		stbi_image_free(data);
		return true;
	}

	//hides faces that are touching to make render more efficient;
	void createSurfaceList() {
		surfaces.clear();
		for (int s = 0; s < 6; s++) {
			surfaces.push_back(std::vector<Surface>());
		}

		for (int i = 0; i < loadedBlocks.size(); i++) {
			for (int j = 0; j < loadedBlocks[i].size(); j++) {
				for (int s = 0; s < 6; s++) {
					surfaces[s].push_back(Surface(loadedBlocks[i][j], &texture[i][s], loadedBlocks[i][j]->blockType, s));
				}
			}
		}
	}

	int renderFrame() {
		if (glfwWindowShouldClose(window)) {
			return 0;
		}

		//handle input
		//processInput(window, &camera);

		//render
		//clear the screen and start next frame
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//for every loaded surface process and draw them
		for (int i = 0; i < surfaces.size(); i++) {
			//set next texture to be rendered
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, *surfaces[i][0].texture);
			
			//activate shader
			ourShader.use();

			ourShader.setMat4("projection", camera.projection);
			ourShader.setMat4("view", camera.update());
			
			glBindVertexArray(VBO[i]);
			glBindVertexArray(VAO[i]);
			//use individual settings of each block
			for (int x = 0; x < surfaces[i].size(); x++) {
				//set next texture to be rendered
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, *surfaces[i][x].texture);

				// calculate the model matrix for each object and pass it to shader before drawing
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, (*surfaces[i][x].block).pos);
				ourShader.setMat4("model", model);

				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			
		}
		

		//swap the buffers for smooth frames and poll events
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