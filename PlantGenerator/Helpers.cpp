#include "Helpers.h"

// Clamp helpers
float clamp(float& value, float low, float high) {
    return (std::max(low, std::min(value, high)));
}
int clamp(int& value, float low, float high) {
    return (value < low ? low : (value > high ? high : value));
}

// Function to generate a random float in the range [min, max]
float getRandomFloat(float min, float max) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed); // Seed the generator
    std::uniform_real_distribution<float> dis(min, max); // Define the range

    return dis(gen); // Generate the random number
}

// 3D angle helpers
float normalRand(float mean, float variance) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::default_random_engine generator(seed);
    std::normal_distribution<float> distribution(mean, variance);
    return distribution(generator);
}

// Function to generate a random angle from a uniform distribution
float uniformRand(float min, float max) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::default_random_engine generator(seed);
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
}

// Function to convert spherical coordinates to Cartesian coordinates
glm::vec3 sphericalToCartesian(float theta, float phi) {
    float x = sin(theta) * cos(phi);
    float z = sin(theta) * sin(phi);
    float y = cos(theta);
    return glm::vec3(x, y, z);
}