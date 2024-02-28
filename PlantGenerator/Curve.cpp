#include "Curve.h"

MeshData Curve::calculateMesh(int aSubdivisions, float startWidth, float endWidth, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2)
{
	Texture textures[]
	{
		Texture("terrible.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE)
	};

	// Calculate vertices
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;

	// Note: Rings refer to the loops on the path, Segments refer to the faces connecting the rings.

	int nbRings = getEdges()+1;
	const float angleStep = 2.0f * glm::pi<float>() / float(aSubdivisions);

	// All Rings
	for (int ringIndex = 0; ringIndex < nbRings; ringIndex++)
	{
		// Note: The demonitator is (nbRings-1) rather than nbRings because we want to find the fraction based on edges rather than rings to
		// ensure that the number of segments = the number of edges on the Bezier Curve. The same goes for some comparisons of ringIndex
		// where to convert from index to the ring number we add 1.
		// t is the normalized value for how far along the curve we are.
		float t = (float)ringIndex / (float)(nbRings - 1);
		glm::vec3 startPosition = Evaluate(t);
		// If we are on the final ring there is no next ring, so we use the previous one instead
		glm::vec3 endPosition =	Evaluate((ringIndex+1) / (float)(nbRings-1));
		glm::vec3 direction;
		// For the first and last rings we make them look towards the control points, this ensures consistent start/end points no matter the detail
		if (ringIndex + 1 == nbRings) direction = -glm::normalize(c2);
		else if (ringIndex == 0) direction = glm::normalize(c1);
		// The aim of this is to copy the prior ring's rotation
		else direction = glm::normalize(endPosition - startPosition);
		// glm::lookAt is a function designed for cameras to use, due to how cameras work we must inverse this vector for it to work for regular positions.
		auto ringRotation = glm::lookAt(glm::vec3(0.0f), direction, glm::vec3(1.0f, 0.0f, 0.0f));
		ringRotation = glm::inverse(ringRotation);
		
		float currentAngle = 0.0f;
		float prevY = std::cos(0.0f);
		float prevZ = std::sin(0.0f);

		// Create a ring
		for (int i = 0; i <= aSubdivisions; i++)
		{
			if (i != aSubdivisions)
			{
				currentAngle = angleStep * i;
				float x = std::sin(currentAngle);
				float y = std::cos(currentAngle);

				// Add the vertices for the ring 
				// They are first rotated to look at the next ring on the curve, then translated to the appropriate start position.
				float width = t * endWidth + (1 - t) * startWidth;
				glm::vec3 vert_dir = glm::vec3(ringRotation * glm::vec4((glm::vec3(x * width, y * width, 0.0f)), 0.0f));
				vertices.push_back(Vertex{ vert_dir + startPosition, glm::normalize(vert_dir), glm::vec3((float)ringIndex / (float)(nbRings - 1)), glm::vec2((float)ringIndex / (float)(nbRings - 1))});

				// Add the indices (face representations) for the segment.
				// We do not create faces on the first loop as we only have 1 vertex
				if (i > 0 && ringIndex > 0)
				{
					// Offset is the index offset for previous ring and slices
					int offset = (i - 1) + (ringIndex-1) * aSubdivisions;

					// 1a 0b 0a
					// 1a 1b 0b
					indices.push_back((GLuint)1 + offset); indices.push_back((GLuint)0 + offset + aSubdivisions); indices.push_back((GLuint)0 + offset);
					indices.push_back((GLuint)1 + offset); indices.push_back((GLuint)1 + offset + aSubdivisions); indices.push_back((GLuint)0 + offset + aSubdivisions);
				}
			}
			else if (ringIndex > 0)
			{
				// Rather than adding extra unnecessary vertices, we instead attach the last faces to the first vertices,
				// hence we loop N+1 times as we cannot create these faces on the first loop
				// TL;DR creating the 2 faces for the last and first vertices in the segment's two rings.

				int ringOffset = (ringIndex-1) * aSubdivisions;
				int offset = (i - 1);

				// 1a 0b 0a
				// 1a 1b 0b
				indices.push_back((GLuint)ringOffset); indices.push_back((GLuint)ringOffset + aSubdivisions + offset); indices.push_back((GLuint)offset + ringOffset);
				indices.push_back((GLuint)ringOffset); indices.push_back((GLuint)ringOffset + aSubdivisions); indices.push_back((GLuint)ringOffset + aSubdivisions + offset);
			}
		}
	}

	// Store Texture Data
	std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));

	// return Mesh Data
	return (MeshData{ vertices, indices, tex });
}