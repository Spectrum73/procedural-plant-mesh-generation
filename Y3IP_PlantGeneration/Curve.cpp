#include "Curve.h"

MeshData Curve::calculateMesh(int aSubdivisions, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2)
{
	Texture textures[]
	{
		Texture("terrible.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE)
	};

	// Calculate vertices
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;

	int segments = GetDetail()+1;
	const float angleStep = 2.0f * glm::pi<float>() / float(aSubdivisions);

	// All Segments
	for (int segmentIndex = 0; segmentIndex <= segments; segmentIndex++)
	{
		// Create a segment
		glm::vec3 startPosition = Evaluate(segmentIndex / (float)segments);
		glm::vec3 endPosition =	segmentIndex == segments ? Evaluate(segmentIndex-1 / (float)segments) : Evaluate((segmentIndex+1) / (float)segments);
		glm::vec3 direction = glm::normalize(endPosition - startPosition);
		if (segmentIndex == segments) direction = -direction;
		auto segmentRotation = glm::lookAt(glm::vec3(0.0f), direction, glm::vec3(0.0f, 1.0f, 0.0f));

		
		float currentAngle = 0.0f;
		float prevY = std::cos(0.0f);
		float prevZ = std::sin(0.0f);
		for (int i = 0; i <= aSubdivisions; i++)
		{
			if (i != aSubdivisions)
			{
				currentAngle = angleStep * i;
				//std::cout << currentAngle << std::endl;
				float y = std::cos(currentAngle);
				float z = std::sin(currentAngle);

				// Add the vertices 
				vertices.push_back(Vertex{ glm::vec3(segmentRotation * glm::vec4((glm::vec3(0.0f, y * CURVE_WIDTH, z * CURVE_WIDTH)), 0.0f)) + startPosition, glm::vec3(0.0f, y, z), glm::vec3(0.0f), glm::vec2(0.0f)});

				// Add the indices (face representations)
				// We do not create faces on the first loop as we only have 2 vertices
				if (i > 0 && segmentIndex > 0)
				{
					// Offset is the index offset for previous segments and slices
					int offset = (i - 1) + (segmentIndex-1) * aSubdivisions;

					// 0a 0b 1a
					// 1a 1b 0b
					indices.push_back((GLuint)0 + offset); indices.push_back((GLuint)0 + offset + aSubdivisions); indices.push_back((GLuint)1 + offset);
					indices.push_back((GLuint)1 + offset); indices.push_back((GLuint)1 + offset + aSubdivisions); indices.push_back((GLuint)0 + offset + aSubdivisions);
				}
			}
			else if (segmentIndex > 0)
			{
				// Rather than adding extra unnecessary vertices, we instead attach the last faces to the first vertices,
				// hence we loop N+1 times as we cannot create these faces on the first loop

				int segmentOffset = (segmentIndex-1) * aSubdivisions;
				int offset = (i - 1);

				// 0a 0b 1a
				// 1a 1b 0b
				indices.push_back((GLuint)offset + segmentOffset);  indices.push_back((GLuint)segmentOffset + aSubdivisions + offset); indices.push_back((GLuint)segmentOffset);
				indices.push_back((GLuint)segmentOffset); indices.push_back((GLuint)segmentOffset + aSubdivisions); indices.push_back((GLuint)segmentOffset + aSubdivisions + offset);
			}
		}
	}

	// Store Texture Data
	std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));

	// return Mesh Data
	return (MeshData{ vertices, indices, tex });
}