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

//callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Camera *camera);

class GraphicsEngine {

public:

	GLFWwindow* window;

	//shader
	Shader ourShader;

	//cube vertex data
	std::vector<std::vector<unsigned int>> VBO;
	std::vector<std::vector<unsigned int>> VAO;

	//camera
	Camera *camera;

	//block data
	std::vector<BlockType> blockType;
	//this vector is effectivly a way to divide the blocks based on block type for drawing. They are arranged based on chunk position since texture is meaningless now.
	std::vector<std::vector<std::vector<Block*>>> loadedBlocks;

	//chunkmap is the vertex data for each chunk
	std::vector<std::vector<std::vector<float>>> chunkMap;

	//texture data
	std::vector<unsigned int> texture;

	GraphicsEngine(const char* windowName, Camera *cam, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT) {
		//initialize Camera
		camera = cam;

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

		//loadedBlocks.push_back(std::vector<Block*>());
	}

	//textures are in order of the block types as they were added to the manager
	void generateTextures() {
		texture.clear();

		for (int i = 0; i < blockType.size(); i++) {
			texture.push_back(NULL);
			loadTexture(&texture[i], blockType[i].texture);

		}

		ourShader.use();
		ourShader.setInt("texture0", 0);
		if (blockType.size() > 1) {
			ourShader.setInt("texture1", 1);
		}
		if (blockType.size() > 2) {
			ourShader.setInt("texture2", 2);
		}
		if (blockType.size() > 3) {
			ourShader.setInt("texture3", 3);
		}
	}

	bool addBlock(Block *block) {
		//loads the block and puts it in the chunk it is supposed to be rendered in.
		int x = std::floor((*block).pos.x / 16);
		int y = std::floor((*block).pos.z / 16);

		if (y >= chunkMap.size()) {
			int size = (y - chunkMap.size());

			for (int i = 0; i <= size; i++) {
				chunkMap.push_back(std::vector<std::vector<float>>());
				loadedBlocks.push_back(std::vector<std::vector<Block*>>());
				VBO.push_back(std::vector<unsigned int>());
				VAO.push_back(std::vector<unsigned int>());
			}
		}

		if (x >= chunkMap[y].size()) {
			int size = (x - chunkMap[y].size());

			for (int i = 0; i <= size; i++) {
				chunkMap[y].push_back(std::vector<float>());
				loadedBlocks[y].push_back(std::vector<Block*>());
				VBO[y].push_back(NULL);
				VAO[y].push_back(NULL);
			}
		}

		//std::cout << x << " " << y << std::endl;
		loadedBlocks[y][x].push_back(block);

		return true;
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
		//stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
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

	void compileVertices() {
		std::cout << "Compiling vertex data into chunks" << std::endl;

		// set up vertex data (and buffer(s)) and configure vertex attributes for blocks
		float cube[] = {
			//back
			-0.5f, -0.5f, -0.5f,  0.75f, 0.3333f,
			 0.5f, -0.5f, -0.5f,  1.0f, 0.3333f,
			 0.5f,  0.5f, -0.5f,  1.0f, 0.6666f,
			 0.5f,  0.5f, -0.5f,  1.0f, 0.6666f,
			-0.5f,  0.5f, -0.5f,  0.75f, 0.6666f,
			-0.5f, -0.5f, -0.5f,  0.75f, 0.3333f,

			//front
			-0.5f, -0.5f,  0.5f,  0.25f, 0.3333f,
			 0.5f, -0.5f,  0.5f,  0.50f, 0.3333f,
			 0.5f,  0.5f,  0.5f,  0.50f, 0.6666f,
			 0.5f,  0.5f,  0.5f,  0.50f, 0.6666f,
			-0.5f,  0.5f,  0.5f,  0.25f, 0.6666f,
			-0.5f, -0.5f,  0.5f,  0.25f, 0.3333f,

			//left
			-0.5f, -0.5f, -0.5f,  0.0f, 0.3333f,
			-0.5f, -0.5f,  0.5f,  0.25f, 0.3333f,
			-0.5f,  0.5f,  0.5f,  0.25f, 0.6666f,
			-0.5f,  0.5f,  0.5f,  0.25f, 0.6666f,
			-0.5f,  0.5f, -0.5f,  0.0f, 0.6666f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.3333f,

			//right
			 0.5f, -0.5f, -0.5f,  0.50f, 0.3333f,
			 0.5f, -0.5f,  0.5f,  0.75f, 0.3333f,
			 0.5f,  0.5f,  0.5f,  0.75f, 0.6666f,
			 0.5f,  0.5f,  0.5f,  0.75f, 0.6666f,
			 0.5f,  0.5f, -0.5f,  0.50f, 0.6666f,
			 0.5f, -0.5f, -0.5f,  0.50f, 0.3333f,

			 //bottom
			-0.5f, -0.5f, -0.5f,  0.25f, 0.0f,
			 0.5f, -0.5f, -0.5f,  0.50f, 0.0f,
			 0.5f, -0.5f,  0.5f,  0.50f, 0.3333f,
			 0.5f, -0.5f,  0.5f,  0.50f, 0.3333f,
			-0.5f, -0.5f,  0.5f,  0.25f, 0.3333f,
			-0.5f, -0.5f, -0.5f,  0.25f, 0.0f,

			//top
			-0.5f,  0.5f, -0.5f,  0.25f, 0.6666f,
			 0.5f,  0.5f, -0.5f,  0.50f, 0.6666f,
			 0.5f,  0.5f,  0.5f,  0.50f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0.50f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.25f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.25f, 0.6666f
		};

		//compile values
		//float vertices[138240];
		
		int totalBlockCount = 0;
		for (int _y = 0; _y < chunkMap.size(); _y++) {
			for (int _x = 0; _x < chunkMap[_y].size(); _x++) {
				totalBlockCount += (loadedBlocks[_y][_x].size());
			}
		}

		int blockProgress = 0;
		int progressCounter = 0;

		for (int _y = 0; _y < chunkMap.size(); _y++) {
			for (int _x = 0; _x < chunkMap[_y].size(); _x++) {
				int count = 0;

				for (int i = 0; i < loadedBlocks[_y][_x].size(); i++) {
					count = 0;
					for (int x = 0; x < sizeof(cube) / sizeof(cube[0]); x++) {
						//std::cout << vertexCount << " " << x  << " " << count << std::endl;
						if (count == 0) {
							chunkMap[_y][_x].push_back((*loadedBlocks[_y][_x][i]).pos.x + cube[x]);
						}
						else if (count == 1) {
							chunkMap[_y][_x].push_back((*loadedBlocks[_y][_x][i]).pos.y + cube[x]);
						}
						else if (count == 2) {
							chunkMap[_y][_x].push_back((*loadedBlocks[_y][_x][i]).pos.z + cube[x]);
						}
						else if (count == 3) {
							//tex coords
							chunkMap[_y][_x].push_back(cube[x]);
						}
						else if (count == 4) {
							//tex coords and also add which texture this is reffering to
							chunkMap[_y][_x].push_back(cube[x]);

							//test and select texture option
							chunkMap[_y][_x].push_back((*loadedBlocks[_y][_x][i]).id);
						}

						count += 1;
						count %= 5;
					}

					//display loading progress
					blockProgress += 1;

					if (floor((float(blockProgress) / totalBlockCount) * 100) > progressCounter) {
						progressCounter = floor((float(blockProgress) / totalBlockCount) * 100);
						std::cout << "\rCompiling and loading verticies: " << progressCounter << "%";
					}
				}

				glGenVertexArrays(1, &VAO[_y][_x]);
				glGenBuffers(1, &VBO[_y][_x]);

				glBindVertexArray(VAO[_y][_x]);

				glBindBuffer(GL_ARRAY_BUFFER, VBO[_y][_x]);
				glBufferData(GL_ARRAY_BUFFER, chunkMap[_y][_x].size() * sizeof(float), chunkMap[_y][_x].data(), GL_STATIC_DRAW);

				// position attribute
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				// texture coord attribute
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);
				// texture label attribute
				glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
				glEnableVertexAttribArray(2);

				//std::cout << vertexCount / (6*6*6) << std::endl;
				//std::cout << loadedBlocks[_y][_x].size() << std::endl;
			}
		}

		std::cout << std::endl;
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
		//std::cout << "bind" << std::endl;
		for (int _y = 0; _y < chunkMap.size(); _y++) {
			for (int _x = 0; _x < chunkMap[_y].size(); _x++) {

				glBindVertexArray(VBO[_y][_x]);
				glBindVertexArray(VAO[_y][_x]);

				//std::cout << "texture" << std::endl;
				//set next texture to be rendered
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture[0]);
				if (blockType.size() > 1) {
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, texture[1]);
				}
				if (blockType.size() > 2) {
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, texture[2]);
				}
				if (blockType.size() > 3) {
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, texture[3]);
				}

				//std::cout << "shader" << std::endl;
				//activate shader
				ourShader.use();

				//std::cout << "proj view" << std::endl;
				ourShader.setMat4("projection", (*camera).projection);
				ourShader.setMat4("view", (*camera).update());

				//std::cout << "model" << std::endl;
				glm::mat4 model = glm::mat4(1.0f);
				ourShader.setMat4("model", model);

				//std::cout << "draw" << std::endl;
				glDrawArrays(GL_TRIANGLES, 0, 36 * loadedBlocks[_y][_x].size());
			}
		}

		/*
		//for every loaded surface process and draw them
		for (int i = 0; i < loadedBlocks.size(); i++) {
			//set next texture to be rendered
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture[i]);

			//activate shader
			ourShader.use();

			ourShader.setMat4("projection", (*camera).projection);
			ourShader.setMat4("view", (*camera).update());

			//use individual settings of each block
			for (int x = 0; x < loadedBlocks[i].size(); x++) {

				// calculate the model matrix for each object and pass it to shader before drawing
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, (*loadedBlocks[i][x]).pos);
				ourShader.setMat4("model", model);

				glDrawArrays(GL_TRIANGLES, 0, 36);
			}

		}
		*/


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