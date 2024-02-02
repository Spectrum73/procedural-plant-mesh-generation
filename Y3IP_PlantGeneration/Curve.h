#ifndef CURVE_H
#define CURVE_H

#include "Mesh.h"
#include "Bezier.h"
#include <vector>

// Refers to how many vertices will represent the circumference of each segment
#define CURVE_SUBDIVISIONS 16
#define CURVE_WIDTH 0.1f

// Not to be confused with Bezier Curves

// A 3D cylindrical representation of a Bezier Curve.
class Curve : public Bezier, public Mesh {
private:
	MeshData calculateMesh(int aSubdivisions, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2);
public:
	Curve(int aSubdivisions, int aSegments, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2) : Bezier(aSegments, a, b, c1, c2), Mesh(calculateMesh(aSubdivisions, a, b, c1, c2)){}
	using Mesh::Draw; // To prevent ambiguity of Bezier's Draw // TODO : This could rename it to DrawBezier so both are accessible
};

#endif