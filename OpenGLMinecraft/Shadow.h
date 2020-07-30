#ifndef SHADOW_H
#define SHADOW_H

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

#include <vector>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shadow {
public:
	Shader shader;

	MinecraftManager *minecraft;

	Light *light;

	unsigned int depthMapFBO;
	const unsigned int width = 1024 * 16, height = 1024 * 16;
	unsigned int depthMap;

	float farPlane = 1000;
	float nearPlane = 0.1f;
	float fov = 90.0f;

	glm::mat4 lightSpaceMatrix;

	//0 is ortho
	//1 is perspective
	int projectionType;

	Shadow() {
		int projectionType = 1;
	}

	Shadow(int type) {
		projectionType = type;
	}

	void setup(const char* vs, const char* fs, MinecraftManager *minecraft, Light *light) {
		shader = Shader(vs, fs);
		this->minecraft = minecraft;
		this->light = light;

		setupDepthBuffer();
	}

	//size is the resolution of the shadow
	void setupDepthBuffer() {
		glGenFramebuffers(1, &depthMapFBO);

		//create an image representing depth buffer
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		//bind the buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glEnable(GL_DEPTH_TEST);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//setup shader
		//shader.use();
		//shader.setInt("", 0);
	}

	//calculate the depthmap
	void calculateShadows() {
		//render depthmap
		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

		//fixes the peter panning effect
		//glCullFace(GL_FRONT);

		//refresh buffer and then render the depthmap to it
		glClear(GL_DEPTH_BUFFER_BIT);
		//render
		glm::mat4 lightProjection = glm::perspective(glm::radians(fov), float(width) / float(height), nearPlane, farPlane);
		//glm::mat4 lightProjection = glm::ortho(-int(chunkMap.size()) * 16.0f * 2, chunkMap.size() * 16.0f * 2, -int(chunkMap.size()) * 16.0f * 2, chunkMap.size()*16.0f * 2, -int(chunkMap.size()) * 16.0f * 2, chunkMap.size()*16.0f * 2);
		glm::mat4 lightView = glm::lookAt((*light).pos, glm::vec3(float(int((*minecraft).chunkMap.size()) * 16 / 2), (*light).pos.y / 2, float(int((*minecraft).chunkMap.size()) * 16 / 2)), glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::mat4 lightView = glm::lookAt(light.pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::mat4 lightView = glm::lookAt(glm::vec3(1,1,1), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProjection * lightView;

		//projectionType = 1;

		//start the main render sequence
		shader.use();
		shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		shader.setFloat("near_plane", nearPlane);
		shader.setFloat("far_plane", farPlane);
		shader.setInt("projType", projectionType);

		//std::cout << "model" << std::endl;
		glm::mat4 model = glm::mat4(1.0f);
		shader.setMat4("model", model);

		//render
		(*minecraft).render();

		//reset cull face
		//glCullFace(GL_BACK);

		//reset buffers
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glViewport(0, 0, 1600, 900);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//std::cout << shadow.depthMap << std::endl;
	}
};

#endif