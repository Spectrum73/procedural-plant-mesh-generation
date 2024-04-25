#ifndef HELPERS_H
#define HELPERS_H

#include <iostream>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <chrono>

// Clamp helpers
float clamp(float& value, float low, float high);
int clamp(int& value, float low, float high);

// Function to generate a random float in the range [min, max]
float getRandomFloat(float min, float max);

// 3D angle helpers
float normalRand(float mean, float variance);

// Function to generate a random angle from a uniform distribution
float uniformRand(float min, float max);

// Function to convert spherical coordinates to Cartesian coordinates
glm::vec3 sphericalToCartesian(float theta, float phi);

#endif