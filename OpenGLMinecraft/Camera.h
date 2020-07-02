#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include "BlockType.h"

#include <string>
#include <iostream>

class Camera {
public:
	//screen sizes
	unsigned int WIDTH, HEIGHT;

	//shader linked to camera
	//Shader *shader;

	//camera values
	glm::vec3 pos;
	glm::vec3 rot;
	glm::mat4 view, projection;

	//blank camera
	Camera() {

	}

	Camera(const unsigned int WIDTH, const unsigned int HEIGHT, glm::vec3 startPos) {
		//link the shaders
		//*this->shader = *shader;

		//make identity matrix
		view = glm::mat4(1.0f);
		projection = glm::mat4(1.0f);

		//set variables
		this->WIDTH = WIDTH;
		this->HEIGHT = HEIGHT;

		//set camera type and variables
		projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

		//set position and rotation
		pos = startPos;

		rot = glm::vec3(0, 0, 0);
	}

	void setPos(glm::vec3 newPos) {
		pos = newPos;
	}

	void move(glm::vec3 translation) {
		pos.x += translation.x;
		pos.y += translation.y;
		pos.z += translation.z;
	}

	glm::mat4 update() {
		view = glm::mat4(1.0f);
		view = glm::translate(view, pos);

		return view;
	}

	//Not finished
	/*
	void use() {
		*shader.setMat4("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		*shader.setMat4("view", view);
	}
	*/
};

#endif
