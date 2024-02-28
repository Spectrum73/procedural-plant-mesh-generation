#include "RandomNumberGeneration.h"

// Function to generate a random float in the range [min, max]
float getRandomFloat(float min, float max) {
    std::random_device rd; // Obtain a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    std::uniform_real_distribution<float> dis(min, max); // Define the range

    return dis(gen); // Generate the random number
}