#ifndef BEZIER_H
#define BEZIER_H

#include <glm/glm.hpp>
#include "VAO.h"
#include "EBO.h"
#include "Camera.h"

#define BEZIER_DEFAULT_EDGES 32

class Bezier 
{
public:
    glm::vec3 Evaluate(float t);  // Parameter 0 <= t <= 1
    Bezier(int aDetail, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2);

    VAO VAO;

    // Draws the curve (Named DrawBezier so it isn't confused with Draw() from Mesh when inherited in Curve)
    void DrawBezier(Shader& shader, Camera& camera);
    void SetEdges(int detail);
    int GetEdges() const { return edges; }
private:
    glm::vec3 A;  // Start value
    glm::vec3 B;  // Ending value
    glm::vec3 C1;  // First control value
    glm::vec3 C2;  // Second control value
    int edges = BEZIER_DEFAULT_EDGES;
};

#endif // !BEZIER_H
