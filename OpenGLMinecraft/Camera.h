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

	//camera calc values
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Right;

	bool firstMouse = true;
	float yaw = 90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float pitch = 0.0f;
	float lastX = 1600.0f / 2.0;
	float lastY = 900.0f / 2.0;
	float fov = 45.0f;

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
		projection = glm::perspective(glm::radians(fov), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

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
		view = glm::lookAt(pos, pos + Front, Up);

		return view;
	}

	//allows for easy camera movement
	void updateCameraVectors()
	{
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, glm::vec3(0.0f, 1.0f, 0.0f)));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
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
