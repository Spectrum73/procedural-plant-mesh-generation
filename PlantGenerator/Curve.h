#ifndef CURVE_H
#define CURVE_H

#include "Mesh.h"
#include "Bezier.h"
#include <vector>

// Refers to how many vertices will represent the circumference of each segment
#define CURVE_SUBDIVISIONS 16
#define CURVE_DEFAULT_WIDTH 0.1f

// Not to be confused with Bezier Curves

// A 3D cylindrical representation of a Bezier Curve.
class Curve : public Bezier, public Mesh {
private:
	MeshData calculateMesh(int aSubdivisions, float startWidth, float endWidth, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2, bool aCapped = false);
public:
	Curve(int aSubdivisions, int aSegments, float startWidth, float endWidth, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2) : Bezier(aSegments, a, b, c1, c2), Mesh(calculateMesh(aSubdivisions, startWidth, endWidth, a, b, c1, c2, true)){}
	Curve(int aSubdivisions, int aSegments, float startWidth, float endWidth, Bezier &bezier) 
		: Bezier(bezier), Mesh(calculateMesh(aSubdivisions, startWidth, endWidth, bezier.getPoint(0), bezier.getPoint(1), bezier.getControlPoint(0), bezier.getControlPoint(1), true)) {}
};

#endif