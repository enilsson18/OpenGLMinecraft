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

	const unsigned int *SCR_WIDTH;
	const unsigned int *SCR_HEIGHT;

	//shader
	Shader ourShader;

	//cube vertex data
	std::vector<std::vector<unsigned int>> VBO;
	std::vector<std::vector<unsigned int>> VAO;

	//shadows
	struct Shadow {
		Shader shader;

		unsigned int depthMapFBO;
		const unsigned int width = 1024 * 10, height = 1024 * 10;
		unsigned int depthMap;

		glm::mat4 lightSpaceMatrix;
	};

	Shadow shadow;

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

	//light that we can move around
	struct Light {
		glm::vec3 pos;
		glm::vec3 color;

		unsigned int VBO;
		unsigned int VAO;

		Shader shader;

		
	};

	Light light;

	GraphicsEngine(const char* windowName, Camera *cam, const unsigned int *scr_WIDTH, const unsigned int *scr_HEIGHT) {
		//initialize Camera
		camera = cam;

		//set standard light pos
		light.pos = glm::vec3(25, 25, 25);
		light.color = glm::vec3(1, 1, 1);

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

		// build and compile our shader program
		ourShader = Shader("resources/shaders/6.3.coordinate_systems.vs", "resources/shaders/6.3.coordinate_systems.fs");
		light.shader = Shader("resources/shaders/light_cube.vs", "resources/shaders/light_cube.fs");
		shadow.shader = Shader("resources/shaders/light_cube.vs", "resources/shaders/shadow_depth.fs");

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

		setupDepthBuffer();
	}

	//size is the resolution of the shadow
	void setupDepthBuffer() {
		glGenFramebuffers(1, &shadow.depthMapFBO);

		//create an image representing depth buffer
		glGenTextures(1, &shadow.depthMap);
		glBindTexture(GL_TEXTURE_2D, shadow.depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow.width, shadow.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//bind the buffer
		glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow.depthMap, 0);
		//glDrawBuffer(GL_NONE);
		//glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	//calculate the depthmap
	void calculateShadows() {
		//render depthmap
		glViewport(0, 0, shadow.width, shadow.height);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
		//refresh buffer and then render the depthmap to it
		//glClear(GL_DEPTH_BUFFER_BIT);
		//render
		//glm::mat4 lightProjection = glm::ortho(-int(chunkMap.size()) * 16.0f, chunkMap.size() * 16.0f, -int(chunkMap.size()) * 16.0f, chunkMap.size()*16.0f, 0.1f, 300.0f);
		glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 300.0f);
		//glm::mat4 lightView = glm::lookAt(light.pos, glm::vec3(float(int(chunkMap.size())*16/2), 0.0f, float(int(chunkMap.size()) * 16/2)), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightView = glm::lookAt(light.pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = glm::mat4(1.0f) * lightProjection * lightView;

		//start the main render sequence
		shadow.shader.use();
		shadow.shader.setMat4("lightSpaceMatrix",lightSpaceMatrix);
		shadow.lightSpaceMatrix = lightSpaceMatrix;
		// draw blocks
		for (int _y = 0; _y < chunkMap.size(); _y++) {
			for (int _x = 0; _x < chunkMap[_y].size(); _x++) {

				glBindVertexArray(VBO[_y][_x]);
				glBindVertexArray(VAO[_y][_x]);

				//std::cout << "model" << std::endl;
				glm::mat4 model = glm::mat4(1.0f);
				shadow.shader.setMat4("model", model);

				//std::cout << "draw" << std::endl;
				glDrawArrays(GL_TRIANGLES, 0, 36 * loadedBlocks[_y][_x].size());
			}
		}

		//reset buffers
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, *SCR_WIDTH, *SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//std::cout << shadow.depthMap << std::endl;
	}

	//add block types and a new column to insert block data
	void addBlockType(BlockType blockType) {
		this->blockType.push_back(blockType);

		//loadedBlocks.push_back(std::vector<Block*>());
	}

	//textures are in order of the block types as they were added to the manager
	void generateTextures() {
		texture.clear();

		ourShader.use();
		for (int i = 0; i < blockType.size(); i++) {
			texture.push_back(NULL);
			loadTexture(&texture[i], blockType[i].texture);

			ourShader.setInt("texture" + std::to_string(i), i);
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
		//it is stored like (posx, pos y, pos z, texcoord x, texcoord y, normal x, normal y, normal z)
		float cube[] = {
			//back
			-0.5f, -0.5f, -0.5f, 0.75f, 0.3333f, 0.0f,  0.0f, -1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 0.3333f, 0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 0.6666f, 0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 0.6666f, 0.0f,  0.0f, -1.0f,
			-0.5f,  0.5f, -0.5f, 0.75f, 0.6666f, 0.0f,  0.0f, -1.0f,
			-0.5f, -0.5f, -0.5f, 0.75f, 0.3333f, 0.0f,  0.0f, -1.0f,

			//front
			-0.5f, -0.5f,  0.5f,  0.25f, 0.3333f,  0.0f,  0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  0.50f, 0.3333f,  0.0f,  0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0.50f, 0.6666f,  0.0f,  0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0.50f, 0.6666f,  0.0f,  0.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.25f, 0.6666f,  0.0f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.25f, 0.3333f,  0.0f,  0.0f, 1.0f,

			//left
			-0.5f, -0.5f, -0.5f,  0.0f, 0.3333f,  -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f,  0.5f, 0.25f, 0.3333f,  -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f, 0.25f, 0.6666f,  -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f, 0.25f, 0.6666f,  -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 0.6666f,  -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.3333f,  -1.0f,  0.0f,  0.0f,

			//right
			 0.5f, -0.5f, -0.5f,  0.50f, 0.3333f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.75f, 0.3333f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.75f, 0.6666f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.75f, 0.6666f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.50f, 0.6666f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.50f, 0.3333f,  1.0f,  0.0f,  0.0f,

			 //bottom
			-0.5f, -0.5f, -0.5f,  0.25f, 0.0f,    0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.50f, 0.0f,    0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.50f, 0.3333f, 0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.50f, 0.3333f, 0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.25f, 0.3333f, 0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.25f, 0.0f,    0.0f, -1.0f,  0.0f,

			//top
			-0.5f,  0.5f, -0.5f,  0.25f, 0.6666f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.50f, 0.6666f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.50f, 1.0f,     0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.50f, 1.0f,     0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.25f, 1.0f,     0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.25f, 0.6666f,  0.0f,  1.0f,  0.0f
		};

		//compile values
		//float vertices[138240];

		//make the light cube
		glGenVertexArrays(1, &light.VAO);
		glGenBuffers(1, &light.VBO);
		glBindVertexArray(light.VAO);

		glBindBuffer(GL_ARRAY_BUFFER, light.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		
		//make the blocks
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
						else if (count >= 3 && count < 7) {
							//tex coords
							chunkMap[_y][_x].push_back(cube[x]);
						}
						else if (count == 7) {
							//tex coords and also add which texture this is reffering to
							chunkMap[_y][_x].push_back(cube[x]);

							//test and select texture option
							chunkMap[_y][_x].push_back((*loadedBlocks[_y][_x][i]).id);
						}

						count += 1;
						count %= 8;
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
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				// texture coord attribute
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);
				//normal vector
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(5 * sizeof(float)));
				glEnableVertexAttribArray(2);
				// texture label attribute
				glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));
				glEnableVertexAttribArray(3);

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

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//handle input
		//processInput(window, &camera);

		//render
		//clear the screen and start next frame
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		calculateShadows();

		//draw light source
		light.shader.use();
		light.shader.setMat4("projection", (*camera).projection);
		light.shader.setMat4("view", (*camera).update());
		glm::mat4 lightModel(1);
		lightModel = glm::translate(lightModel, light.pos);
		lightModel = glm::scale(lightModel, glm::vec3(1)); //change cube size
		light.shader.setMat4("model", lightModel);

		glBindVertexArray(light.VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		// draw blocks
		for (int _y = 0; _y < chunkMap.size(); _y++) {
			for (int _x = 0; _x < chunkMap[_y].size(); _x++) {

				glBindVertexArray(VBO[_y][_x]);
				glBindVertexArray(VAO[_y][_x]);

				//shadow.shader.setInt("shadowMap", 1);

				//std::cout << "texture" << std::endl;
				//set next texture to be rendered
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, shadow.depthMap);
				if (blockType.size() >= 1) {
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, texture[0]);
				}
				if (blockType.size() >= 2) {
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, texture[1]);
				}
				if (blockType.size() >= 3) {
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, texture[2]);
				}
				if (blockType.size() >= 4) {
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, texture[3]);
				}
				if (blockType.size() >= 5) {
					glActiveTexture(GL_TEXTURE5);
					glBindTexture(GL_TEXTURE_2D, texture[4]);
				}
				if (blockType.size() >= 6) {
					glActiveTexture(GL_TEXTURE6);
					glBindTexture(GL_TEXTURE_2D, texture[5]);
				}

				//std::cout << "shader" << std::endl;
				//activate shader
				ourShader.use();

				//std::cout << "proj view" << std::endl;
				ourShader.setMat4("projection", (*camera).projection);
				ourShader.setMat4("view", (*camera).update());

				//shadows
				ourShader.setMat4("lightSpaceMatrix", shadow.lightSpaceMatrix);

				//lighting
				ourShader.setVec3("viewPos", (*camera).pos);
				ourShader.setVec3("lightPos", light.pos);
				ourShader.setVec3("lightColor", light.color);

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