#ifndef CURVE_H
#define CURVE_H

#include "Mesh.h"
#include "Bezier.h"
#include <vector>

// Refers to how many vertices will represent the circumference of each segment
#define CURVE_SUBDIVISIONS 12

// Not to be confused with Bezier Curves

// A 3D cylindrical representation of a Bezier Curve.
class Curve : public Mesh {
public:
	Curve(int aSubdivisions, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2) : Mesh(calculateMesh(aSubdivisions, a, b, c1, c2)), bezierCurve(a, b, c1, c2) {}
private:
	Bezier bezierCurve;
	MeshData calculateMesh(int aSubdivisions, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2);
};

#endif