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
#include "Quad.h"

#include <vector>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

//NOTICE: as of now gaussian blurring is being put on hold since pcf works very well 
//NOTICE: I may get it working in the future but I would rather focus on things like water and reflections
//NOTICE: I learned a lot in the process of making gaussian blur soft shadows even though it is not complete.

//in order to properly use you must take in two 2d textures to compare with the shader.
//one is a depthmap to see if the fragment the camera is looking at is behind the depthmap value 
//and another to handle blurring of shadows to overlap in the non-shadow areas

class Shadow {
public:
	Shader shader;
	Shader colorShader;
	Shader blurShader;

	MinecraftManager *minecraft;

	Light *light;
	Camera *camera;

	//quad to render to
	Quad quad;

	const unsigned int width = 1024 * 16, height = 1024 * 16;

	unsigned int depthMapFBO;
	unsigned int depthMap;

	//color version black and white of depthMap
	unsigned int depthColorMapFBO;
	unsigned int depthColorMap;

	//difference is that depth map is the depth map pure and simple but the shadow map uses gaussian blurring
	//shadow map x is x and the other one is implied to be for the y axis for gaussian smoothing
	unsigned int shadowMapXFBO;
	unsigned int shadowMapX;

	unsigned int shadowMapFBO;
	unsigned int shadowMap;

	float farPlane = 1000;
	float nearPlane = 0.1f;
	float fov = 90.0f;

	glm::mat4 lightSpaceMatrix;

	//status variables
	//0 is ortho
	//1 is perspective
	int projectionType;

	//significantly more resources used
	bool gaussianSmoothing = false;

	Shadow() {
		projectionType = 1;
	}

	Shadow(int type) {
		projectionType = type;
	}

	void setup(const char* vs, const char* fs, MinecraftManager *minecraft, Camera *camera, Light *light) {
		shader = Shader(vs, fs);

		blurShader = Shader("resources/shaders/gausian_blur.vs", "resources/shaders/gausian_blur.fs");

		colorShader = Shader("resources/shaders/shadow_color.vs", "resources/shaders/shadow_color.fs");

		this->minecraft = minecraft;
		this->camera = camera;
		this->light = light;

		setupDepthBuffer();

		if (gaussianSmoothing) {
			setupDepthColorBuffer();
			setupBlurBuffer();
		}
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


		if (gaussianSmoothing) {
			//enter the depth buffer colorization phase
			glBindFramebuffer(GL_FRAMEBUFFER, depthColorMapFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			colorShader.use();
			colorShader.setMat4("model", glm::mat4(1.0f));
			colorShader.setMat4("projection", lightProjection);
			colorShader.setMat4("view", lightView);

			//shadows
			colorShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
			colorShader.setFloat("near_plane", nearPlane);
			colorShader.setFloat("far_plane", farPlane);
			colorShader.setInt("projType", projectionType);


			//lighting
			colorShader.setVec3("viewPos", (*camera).pos);
			colorShader.setVec3("lightPos", (*light).pos);
			colorShader.setVec3("lightColor", (*light).color);
			colorShader.setFloat("lightBrightness", (*light).brightness);
			colorShader.setFloat("lightDistance", (*light).distance);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);

			(*minecraft).render();



			//enter the gausian blur buffer phase
			//render the current information to a quad and then send that data to the shader
			//x axis
			glBindFramebuffer(GL_FRAMEBUFFER, shadowMapXFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			blurShader.use();
			blurShader.setInt("stage", 0);
			blurShader.setFloat("textureWidth", width);
			blurShader.setFloat("textureHeight", height);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthColorMap);

			//render to quad
			quad.render();


			//y axis
			glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			blurShader.use();
			blurShader.setInt("stage", 1);
			blurShader.setFloat("textureWidth", width);
			blurShader.setFloat("textureHeight", height);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowMapX);

			//render to quad
			quad.render();
		}
		

		//reset buffers
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glViewport(0, 0, 1600, 900);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//std::cout << shadow.depthMap << std::endl;
	}

	//size is the resolution of the shadow
	void setupDepthBuffer() {
		glGenFramebuffers(1, &depthMapFBO);

		//create an image representing base depth buffer
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
		//glDrawBuffer(GL_NONE);
		//glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void setupDepthColorBuffer() {
		//make the shadow buffer and bind it to quad fbo
		glGenFramebuffers(1, &depthColorMapFBO);

		//create an image representing base depth buffer
		glGenTextures(1, &depthColorMap);
		glBindTexture(GL_TEXTURE_2D, depthColorMap);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//bind the buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthColorMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthColorMap, 0);
		glEnable(GL_DEPTH_TEST);
		//glDrawBuffer(GL_NONE);
		//glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		colorShader.use();
		colorShader.setInt("shadowMap", 0);
	}

	//has two buffers one for the x and one for the y
	void setupBlurBuffer() {
		//x values
		//make the shadow buffer and bind it to quad fbo
		glGenFramebuffers(1, &shadowMapXFBO);

		//create an image representing base depth buffer
		glGenTextures(1, &shadowMapX);
		glBindTexture(GL_TEXTURE_2D, shadowMapX);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//bind the buffer
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapXFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowMapX, 0);
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//y values
		//make the shadow buffer and bind it to quad fbo
		glGenFramebuffers(1, &shadowMapFBO);

		//create an image representing base depth buffer
		glGenTextures(1, &shadowMap);
		glBindTexture(GL_TEXTURE_2D, shadowMap);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//bind the buffer
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowMap, 0);
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		blurShader.use();
		blurShader.setInt("tex", 0);
	}
};

#endif