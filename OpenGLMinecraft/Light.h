#ifndef LIGHT_H
#define LIGHT_H

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

class Light {
public:

	glm::vec3 pos;
	glm::vec3 color;
	//1 is normal
	float brightness;
	//the effective distance of the light source
	float distance;

	unsigned int VBO;
	unsigned int VAO;

	Shader shader;

	Light() {

	}

	Light(glm::vec3 pos, glm::vec3 color) {
		this->pos = pos;
		this->color = color;

		brightness = 1;
		distance = 300;
	}

	//include shader files
	void setup(const char* vs, const char* fs) {

		shader = Shader(vs, fs);

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
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void render(glm::mat4 proj, glm::mat4 view) {
		//draw light source
		shader.use();
		shader.setMat4("projection", proj);
		shader.setMat4("view", view);
		glm::mat4 lightModel(1);
		lightModel = glm::translate(lightModel, pos);
		lightModel = glm::scale(lightModel, glm::vec3(1)); //change cube size
		shader.setMat4("model", lightModel);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
};

#endif