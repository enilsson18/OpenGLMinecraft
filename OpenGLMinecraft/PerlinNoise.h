#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>
#include <ctime>

#include <string>
#include <iostream>

class PerlinNoise {
public:

	//methods
	static std::vector<std::vector<float>> generate(int size) {
		std::vector<glm::vec2> gradientVectorOptions;
		gradientVectorOptions.push_back(glm::vec2(1, 1));
		gradientVectorOptions.push_back(glm::vec2(-1, 1));
		gradientVectorOptions.push_back(glm::vec2(-1, -1));
		gradientVectorOptions.push_back(glm::vec2(1, -1));
		
		srand(time(0));

		//make gradient vector maps
		int subDivisionSize = 1;
		std::vector<std::vector<glm::vec2>> gradientVectors;
		for (int y = 0; y < size; y += 1) {
			gradientVectors.push_back(std::vector<glm::vec2>());
			for (int x = 0; x < size; x += 1) {
				//generates random number between 0 and 3
				gradientVectors[y].push_back(gradientVectorOptions[int(((double)rand() / (RAND_MAX) * gradientVectorOptions.size()))]);
			}
		}

		std::cout << "finished gradient";

		std::vector<std::vector<float>> subDivision;
		for (int y = 0; y < size; y++) {
			//make new line
			subDivision.push_back(std::vector<float>());
			for (int x = 0; x < size; x++) {
				//find direction vectors for each corner
				//oriented
				//1    2
				//3    4
				std::vector<glm::vec2> directionVectors;
				directionVectors.push_back(glm::vec2(0 + (x + 0.5f), 0 + (y + 0.5f)));
				directionVectors.push_back(glm::vec2((x + 0.5f) - (size), 0 + (y + 0.5f)));
				directionVectors.push_back(glm::vec2(0 + (x + 0.5f), (y + 0.5f) - (size)));
				directionVectors.push_back(glm::vec2((x + 0.5f) - (size), (y + 0.5f) - (size)));

				//dot products
				std::vector<float> dp;
				dp.push_back(glm::dot(gradientVectors[0][0], directionVectors[0]));
				dp.push_back(glm::dot(gradientVectors[0][1], directionVectors[1]));
				dp.push_back(glm::dot(gradientVectors[1][0], directionVectors[2]));
				dp.push_back(glm::dot(gradientVectors[1][1], directionVectors[3]));

				//interpolation step
				float AB = dp[0] + ((x + 0.5f) / size) * (dp[1] - dp[0]);
				float CD = dp[2] + ((x + 0.5f) / size) * (dp[3] - dp[2]);
				subDivision[y].push_back(AB + ((y + 0.5f) / size) * (CD - AB));
			}
		}

		std::cout << "finished subdivision" << std::endl;
		return subDivision;
	}
};

#endif