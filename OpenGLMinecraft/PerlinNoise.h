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

float fade(float num);

class PerlinNoise {
public:
	static int level;

	//methods
	//octaves is the number of subdivision levels and must be cleanly divisible by the size
	//if detail is 1 only one pass will be made and no extra detail will be added
	//detail amplitude is the fraction of amplitude to recursivly be removed
	//octave detail scale must be greater than 1
	static std::vector<std::vector<float>> generateNoise(int size, unsigned int octaves, float amplitude, int detail, int octaveDetailScale, float detailAmplitudeScale) {
		std::vector<std::vector<float>> noiseMap = generate(size, octaves, amplitude, detail, octaveDetailScale, detailAmplitudeScale);
		std::vector<std::vector<float>> newNoiseMap = generate(size, 1, amplitude, 0, 0, 0);
		for (int y = 0; y < noiseMap.size(); y++) {
			for (int x = 0; x < noiseMap.size(); x++) {
				//add the maps together and subtract the amplitude of the detail so the final amplitude remains the same.
				noiseMap[y][x] += (newNoiseMap[y][x]) - amplitude;
			}
		}
	}

	static std::vector<std::vector<float>> generate(int size, unsigned int octaves, float amplitude, int detail, int octaveDetailScale, float detailAmplitudeScale) {

		std::vector<glm::vec2> gradientVectorOptions;
		gradientVectorOptions.push_back(glm::vec2(1, 1));
		gradientVectorOptions.push_back(glm::vec2(-1, 1));
		gradientVectorOptions.push_back(glm::vec2(-1, -1));
		gradientVectorOptions.push_back(glm::vec2(1, -1));

		//ensure octaves are divisible worst case scenario being 1
		while (size % octaves != 0) {
			octaves -= 1;
		}

		int octaveSize = size/octaves;

		srand(time(0));

		//make gradient vector maps
		std::vector<std::vector<glm::vec2>> gradientVectors;
		for (int y = 0; y <= octaves; y += 1) {
			gradientVectors.push_back(std::vector<glm::vec2>());
			for (int x = 0; x <= octaves; x += 1) {
				//generates random number between 0 and 3
				//gradientVectors[y].push_back(gradientVectorOptions[int(((double)rand() / (RAND_MAX) * gradientVectorOptions.size()))]);
				float _x, _y;
				float angle = (double)rand() / (RAND_MAX) * 2 * 3.1415926535f;
				_x = cos(angle);
				_y = sin(angle);

				gradientVectors[y].push_back(glm::vec2(_x, _y));
			}
		}

		//std::cout << "finished gradient" << std::endl;

		std::vector<std::vector<float>> noiseMap;
		for (int _y = 0; _y < octaves; _y++) {
			for (int _x = 0; _x < octaves; _x++) {
				//octave subdivision
				for (int y = 0; y < octaveSize; y++) {
					//make new line
					if (_x == 0) {
						noiseMap.push_back(std::vector<float>());
					}
					for (int x = 0; x < octaveSize; x++) {
						//find direction vectors for each corner
						//oriented
						//1    2
						//3    4
						std::vector<glm::vec2> directionVectors;
						directionVectors.push_back(glm::vec2(0 + (x + 0.5f), 0 + (y + 0.5f)));
						directionVectors.push_back(glm::vec2((x + 0.5f) - (octaveSize), 0 + (y + 0.5f)));
						directionVectors.push_back(glm::vec2(0 + (x + 0.5f), (y + 0.5f) - (octaveSize)));
						directionVectors.push_back(glm::vec2((x + 0.5f) - (octaveSize), (y + 0.5f) - (octaveSize)));

						//dot products
						std::vector<float> dp;
						dp.push_back(glm::dot(gradientVectors[_x][_y], directionVectors[0]));
						dp.push_back(glm::dot(gradientVectors[_x+1][_y], directionVectors[1]));
						dp.push_back(glm::dot(gradientVectors[_x][_y+1], directionVectors[2]));
						dp.push_back(glm::dot(gradientVectors[_x+1][_y+1], directionVectors[3]));

						//interpolation step
						float AB = dp[0] + ((x + 0.5f) / octaveSize) * (dp[1] - dp[0]);
						float CD = dp[2] + ((x + 0.5f) / octaveSize) * (dp[3] - dp[2]);
						//finish interpolation and scale
						float ABCD = ((AB + ((y + 0.5f) / octaveSize) * (CD - AB)) / octaveSize);
						ABCD = ((ABCD + 1) / 2) * amplitude;
						noiseMap[_y*octaveSize + y].push_back(ABCD);
					}
				}
			}
		}

		if (detail > 1 && octaves != size) {
			//increase the number of octaves so more detail can be added
			int newOctaves = octaves * octaveDetailScale;
			while (size % newOctaves != 0) {
				newOctaves += 1;
			}

			std::vector<std::vector<float>> newNoiseMap = generate(size, newOctaves, amplitude * detailAmplitudeScale, detail - 1, octaveDetailScale, detailAmplitudeScale);

			for (int y = 0; y < noiseMap.size(); y++) {
				for (int x = 0; x < noiseMap.size(); x++) {
					//add the maps together and subtract the amplitude of the detail so the final amplitude remains the same.
					noiseMap[y][x] += (newNoiseMap[y][x]) - (amplitude * detailAmplitudeScale);
				}
			}
		}

		//std::cout << "finished subdivision" << std::endl;
		return noiseMap;
	}

	//fade function
	static float fade(float num) {
		return 6 * pow(num, 5) - 15 * pow(num, 4) + 10 * pow(num, 3);
	}
};

#endif