#ifndef MINECRAFTMANAGER_H
#define MINECRAFTMANAGER_H

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
#include "Light.h"
#include "Skybox.h"
//#include "Shadow.h"

#include <vector>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class MinecraftManager {
public:
	Shader shader;

	//cube vertex data
	std::vector<std::vector<unsigned int>> VBO;
	std::vector<std::vector<unsigned int>> VAO;

	//block data
	std::vector<BlockType> blockType;
	//this vector is effectivly a way to divide the blocks based on block type for drawing. They are arranged based on chunk position since texture is meaningless now.
	std::vector<std::vector<std::vector<Block*>>> loadedBlocks;

	//chunkmap is the vertex data for each chunk
	std::vector<std::vector<std::vector<float>>> chunkMap;

	//texture data
	std::vector<unsigned int> texture;

	MinecraftManager() {

	}

	void setup(const char* vs, const char* fs) {
		shader = Shader(vs, fs);

		shader.use();
		shader.setInt("shadowMap", 0);

	}

	//add block types and a new column to insert block data
	void addBlockType(BlockType blockType) {
		this->blockType.push_back(blockType);

		//loadedBlocks.push_back(std::vector<Block*>());
	}

	//textures are in order of the block types as they were added to the manager
	void generateTextures() {
		texture.clear();

		shader.use();
		shader.setInt("shadowMap", 0);
		shader.setInt("shadowBlurMap", 1);

		for (int i = 0; i < blockType.size(); i++) {
			texture.push_back(NULL);
			loadTexture(&texture[i], blockType[i].texture);

			shader.setInt("texture" + std::to_string(i + 1), i + 2);
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
				//sorting for vertices
				std::vector<std::vector<int>> sortedSides;

				//initialize
				for (int i = 0; i < loadedBlocks[_y][_x].size(); i++) {
					sortedSides.push_back(std::vector<int>());
					for (int j = 0; j < 6; j++) {
						sortedSides[i].push_back(1);
					}

					if (sortedSides.size() > 1) {
						for (int j = 0; j < sortedSides.size(); j++) {
							//z axis
							if ((*loadedBlocks[_y][_x][i]).pos + glm::vec3(0.0f, 0.0f, -1.0f) == (*loadedBlocks[_y][_x][j]).pos) {
								sortedSides[i][0] = 0;
								sortedSides[j][1] = 0;
							}
							//x axis
							else if ((*loadedBlocks[_y][_x][i]).pos + glm::vec3(-1.0f, 0.0f, 0.0f) == (*loadedBlocks[_y][_x][j]).pos) {
								sortedSides[i][2] = 0;
								sortedSides[j][3] = 0;
							}
							//y axis
							else if ((*loadedBlocks[_y][_x][i]).pos + glm::vec3(0.0f, -1.0f, 0.0f) == (*loadedBlocks[_y][_x][j]).pos) {
								sortedSides[i][4] = 0;
								sortedSides[j][5] = 0;
							}
						}
					}
				}

				/*
				for (int i = 0; i < loadedBlocks[_y][_x].size(); i++) {
					for (int j = 0; j < loadedBlocks[_y][_x].size(); j++) {
						//z axis
						if ((*loadedBlocks[_y][_x][i]).pos.z - 1 == (*loadedBlocks[_y][_x][j]).pos.z) {
							sortedSides[i][0] = 0;
							sortedSides[j][1] = 0;
						}
						//x axis
						else if ((*loadedBlocks[_y][_x][i]).pos.x - 1 == (*loadedBlocks[_y][_x][j]).pos.x) {
							sortedSides[i][2] = 0;
							sortedSides[j][3] = 0;
						}
						//y axis
						else if ((*loadedBlocks[_y][_x][i]).pos.y - 1 == (*loadedBlocks[_y][_x][j]).pos.y) {
							sortedSides[i][4] = 0;
							sortedSides[j][5] = 0;
						}
					}
				}
				*/

				int count = 0;

				for (int i = 0; i < loadedBlocks[_y][_x].size(); i++) {
					for (int j = 0; j < 6; j++) {
						count = 0;
						//zero means no vertice
						if (sortedSides[i][j] == 1) {

							for (int x = j * ((sizeof(cube) / sizeof(cube[0])) / 6); x < (j + 1) * ((sizeof(cube) / sizeof(cube[0])) / 6); x++) {
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
						}
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

	void renderWorld(glm::mat4 proj, glm::mat4 view, Camera camera, Light light, int projectionType, unsigned int depthMap, float nearPlane, float farPlane, unsigned int colorMap, glm::mat4 lightSpaceMatrix) {
		//std::cout << "shader" << std::endl;
				//activate shader
		shader.use();

		//std::cout << "proj view" << std::endl;
		//shader.setMat4("projection", glm::perspective(glm::radians(90.0f), float(1024) / float(1024), 0.1f, 1000.0f));
		//shader.setMat4("view", glm::lookAt(light.pos, glm::vec3(float(int(chunkMap.size()) * 16 / 2), light.pos.y / 2, float(int(chunkMap.size()) * 16 / 2)), glm::vec3(0.0f, 1.0f, 0.0f)));
		shader.setMat4("projection", proj);
		shader.setMat4("view", view);

		//shadows
		//shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		shader.setFloat("near_plane", camera.nearPlane);
		shader.setFloat("far_plane", camera.farPlane);

		shader.setFloat("shadow_near", nearPlane);
		shader.setFloat("shadow_far", farPlane);
		//shader.setInt("projType", projectionType);
		shader.setInt("pcflevel", 1);

		//lighting
		shader.setVec3("viewPos", camera.pos);
		shader.setVec3("lightPos", light.pos);
		shader.setVec3("lightColor", light.color);
		shader.setFloat("lightBrightness", light.brightness);
		shader.setFloat("lightDistance", light.distance);

		//std::cout << "texture" << std::endl;
				//set next texture to be rendered
				//std::cout << depthMap << std::endl;
		glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, depthMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, colorMap);
		if (blockType.size() >= 1) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, texture[0]);
		}
		if (blockType.size() >= 2) {
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, texture[1]);
		}
		if (blockType.size() >= 3) {
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, texture[2]);
		}
		if (blockType.size() >= 4) {
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, texture[3]);
		}
		if (blockType.size() >= 5) {
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, texture[4]);
		}
		if (blockType.size() >= 6) {
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, texture[5]);
		}

		//std::cout << "model" << std::endl;
		glm::mat4 model = glm::mat4(1.0f);
		shader.setMat4("model", model);

		//default value idk why
		
		glCullFace(GL_FRONT);
		render();
		glCullFace(GL_BACK);
	}

	void render() {
		// draw blocks
		for (int _y = 0; _y < chunkMap.size(); _y++) {
			for (int _x = 0; _x < chunkMap[_y].size(); _x++) {

				//glBindVertexArray(VBO[_y][_x]);
				glBindVertexArray(VAO[_y][_x]);

				//std::cout << "model" << std::endl;
				//glm::mat4 model = glm::mat4(1.0f);
				//shader.setMat4("model", model);

				//std::cout << "draw" << std::endl;
				glDrawArrays(GL_TRIANGLES, 0, 36 * loadedBlocks[_y][_x].size());
				glBindVertexArray(0);
			}
		}
	}
};
#endif