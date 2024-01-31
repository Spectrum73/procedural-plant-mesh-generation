#ifndef BEZIER_H
#define BEZIER_H

#include <glm/glm.hpp>
#include "VAO.h"
#include "Camera.h"

class Bezier 
{
public:
    glm::vec3 Evaluate(float t);  // Parameter 0 <= t <= 1
    Bezier(glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2);

    VAO VAO;

    // Draws the curve
    void Draw(Shader& shader, Camera& camera);
private:
    glm::vec3 A;  // Start value
    glm::vec3 B;  // Ending value
    glm::vec3 C1;  // First control value
    glm::vec3 C2;  // Second control value
};

#endif // !BEZIER_H
